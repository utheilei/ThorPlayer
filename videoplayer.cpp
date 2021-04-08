#include "videoplayer.h"

#include <QDebug>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/pixfmt.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libswresample/swresample.h"
}

#define MAX_AUDIO_FRME_SIZE 48000 * 4

VideoPlayer::VideoPlayer()
{

}

VideoPlayer::~VideoPlayer()
{

}

void VideoPlayer::setFileName(QString &name)
{
    if (!m_nameList.contains(name))
        m_nameList.append(name);

    m_currentFile = name;
}

void VideoPlayer::run()
{
    AVFormatContext *pFormatCtx;
    AVCodecContext *pCodecCtx;
    AVCodec *pCodec;
    AVFrame *pFrame, *pFrameRGB;
    AVPacket *packet;
    struct SwsContext *img_convert_ctx;
    unsigned char *out_buffer;

    int videoStream, i, audioStream;
    int ret, got_picture;

    av_register_all(); //初始化FFMPEG  调用了这个才能正常适用编码器和解码器
    avformat_network_init();

    //Allocate an AVFormatContext.
    pFormatCtx = avformat_alloc_context();

    if (avformat_open_input(&pFormatCtx, m_currentFile.toUtf8().data(), Q_NULLPTR, Q_NULLPTR) != 0) {
        printf("can't open the file.");
        return;
    }

    if (avformat_find_stream_info(pFormatCtx, Q_NULLPTR) < 0) {
        printf("Could't find stream infomation.");
        return;
    }

    videoStream = -1;
    audioStream = -1;

    qDebug() << pFormatCtx->duration / 1000000;

    ///循环查找视频中包含的流信息，直到找到视频类型的流
    ///便将其记录下来 保存到videoStream变量中
    ///这里我们现在只处理视频流  音频流先不管他
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
        }

        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStream = i;
        }
    }

    ///如果videoStream为-1 说明没有找到视频流
    if (videoStream == -1) {
        printf("Didn't find a video stream.");
    }

    if (audioStream == -1) {
        printf("Didn't find a audio Stream.");
    }

    AVCodecContext *audioCodecCtx = pFormatCtx->streams[audioStream]->codec;
    AVCodec *audioCodec = avcodec_find_decoder(audioCodecCtx->codec_id);
    if (audioCodec == Q_NULLPTR) {
        printf("%s", "无法获取audio解码器");
        return;
    }
    //打开解码器
    if (avcodec_open2(audioCodecCtx, audioCodec, Q_NULLPTR) < 0) {
        printf("%s", "无法打开audio解码器");
        return;
    }

    //AVPacket *audioPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
    //解压缩数据
    AVFrame *audioFrame = av_frame_alloc();
    //frame->16bit 44100 PCM 统一音频采样格式与采样率
    SwrContext *swrCtx = swr_alloc();

    //重采样设置参数-------------start
    //输入的采样格式
    enum AVSampleFormat in_sample_fmt = audioCodecCtx->sample_fmt;
    //输出采样格式16bit PCM
    enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    //输入采样率
    int in_sample_rate = audioCodecCtx->sample_rate;
    //输出采样率
    int out_sample_rate = 44100;
    //获取输入的声道布局
    //根据声道个数获取默认的声道布局（2个声道，默认立体声stereo）
    //av_get_default_channel_layout(codecCtx->channels);
    uint64_t in_ch_layout = audioCodecCtx->channel_layout;
    //输出的声道布局（立体声）
    uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;

    swr_alloc_set_opts(swrCtx, out_ch_layout, out_sample_fmt, out_sample_rate,
                       in_ch_layout, in_sample_fmt, in_sample_rate, 0, Q_NULLPTR);
    swr_init(swrCtx);

    //输出的声道个数
    int out_channel_nb = av_get_channel_layout_nb_channels(out_ch_layout);

    //重采样设置参数-------------end

    //16bit 44100 PCM 数据
    uint8_t *audioBuffer = (uint8_t *) av_malloc(MAX_AUDIO_FRME_SIZE);

    int got_frame = 0, audioRet;

    ///查找解码器
    AVStream *stream = pFormatCtx->streams[videoStream];
    pCodecCtx = stream->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);

    if (pCodec == Q_NULLPTR) {
        printf("Codec not found.");
        return;
    }

    ///打开解码器
    if (avcodec_open2(pCodecCtx, pCodec, Q_NULLPTR) < 0) {
        printf("Could not open codec.");
        return;
    }

    pFrame = av_frame_alloc();
    pFrameRGB = av_frame_alloc();

    int y_size = pCodecCtx->width * pCodecCtx->height;

    packet = (AVPacket *) malloc(sizeof(AVPacket)); //分配一个packet
    av_new_packet(packet, y_size); //分配packet的数据

    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height,
                                     AV_PIX_FMT_YUV420P, SWS_BICUBIC, Q_NULLPTR, Q_NULLPTR, Q_NULLPTR);

    out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 4));

    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer,
                         AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 4);

    while (1)
    {
        if (av_read_frame(pFormatCtx, packet) < 0)
        {
            break; //这里认为视频读取完了
        }

        if (packet->stream_index == audioStream) {
            audioRet = avcodec_decode_audio4(audioCodecCtx, audioFrame, &got_frame, packet);
            if (audioRet < 0) {
                printf("audio decode error.");
            }
            //解码一帧成功
            if (got_frame > 0) {
                swr_convert(swrCtx, &audioBuffer, MAX_AUDIO_FRME_SIZE,
                            const_cast<const uint8_t **>(audioFrame->data), audioFrame->nb_samples);

                int audioBufferSize = av_samples_get_buffer_size(Q_NULLPTR, out_channel_nb,
                                                                 audioFrame->nb_samples, out_sample_fmt, 1);

                emit sendAudioBuffer((char*)audioBuffer, audioBufferSize);
            }
        }

        if (packet->stream_index == videoStream) {
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if (ret < 0) {
                printf("decode error.");
                return;
            }
            if (got_picture) {
                sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0,
                                    pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

                double vedio_best_effort_timestamp_second = pFrame->best_effort_timestamp * av_q2d(stream->time_base);

                emit sendBuffer(QSize(pCodecCtx->width, pCodecCtx->height),
                                pFrameRGB->data[0], pFrameRGB->data[1], pFrameRGB->data[2],
                                pFrameRGB->linesize[0], pFrameRGB->linesize[1], pFrameRGB->linesize[2]);
            }
        }
        av_free_packet(packet);
    }
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
}
