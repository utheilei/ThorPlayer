#include "audiohandle.h"

extern "C"
{
#include "libavutil/pixfmt.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libswresample/swresample.h"
}

#include <QDebug>

#define MAX_AUDIO_FRME_SIZE 48000 * 4

AudioHandle::AudioHandle(int id, AVFormatContext *avCodecContext, QObject *parent) : QThread(parent)
{
    m_audioStream = id;
    m_formatCtx = avCodecContext;
}

AudioHandle::~AudioHandle()
{

}

void AudioHandle::run()
{
    AVCodecContext *audioCodecCtx = m_formatCtx->streams[m_audioStream]->codec;
    AVCodec *audioCodec = avcodec_find_decoder(audioCodecCtx->codec_id);
    if (audioCodec == Q_NULLPTR) {
        qDebug() << "not get audio Codec";
        return;
    }
    //打开解码器
    if (avcodec_open2(audioCodecCtx, audioCodec, Q_NULLPTR) < 0) {
        qDebug() << "not open audio Codec";
        return;
    }

    AVPacket *audioPacket = (AVPacket *) av_malloc(sizeof(AVPacket));
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
    qDebug() << out_ch_layout << out_sample_fmt << out_sample_rate
             << "====" << in_ch_layout << in_sample_fmt << in_sample_rate;

//    QAudioFormat fmt; //Qt音频的格式
//    fmt.setSampleRate(in_sample_rate); //采样率
//    fmt.setSampleSize(audioCodecCtx->sample_fmt); //位数
//    fmt.setChannelCount(in_ch_layout);  //声道
//    fmt.setCodec("audio/pcm"); //音频的格式
//    fmt.setByteOrder(QAudioFormat::LittleEndian); //次序
//    fmt.setSampleType(QAudioFormat::SignedInt); //样本的类别

//    m_AudioOutput = new QAudioOutput(fmt);
//    QIODevice *ioDevice = m_AudioOutput->start();

    //16bit 44100 PCM 数据
    uint8_t *audioBuffer = (uint8_t *) av_malloc(MAX_AUDIO_FRME_SIZE);

    int got_frame = 0, audioRet;

    while (av_read_frame(m_formatCtx, audioPacket) >= 0) {
        if (audioPacket->stream_index == m_audioStream) {
            audioRet = avcodec_decode_audio4(audioCodecCtx, audioFrame, &got_frame, audioPacket);
            if (audioRet < 0) {
                printf("audio decode error.");
            }
            //解码一帧成功
            if (got_frame > 0) {
                swr_convert(swrCtx, &audioBuffer, MAX_AUDIO_FRME_SIZE,
                            const_cast<const uint8_t **>(audioFrame->data), audioFrame->nb_samples);

                int audioBufferSize = av_samples_get_buffer_size(Q_NULLPTR, out_channel_nb,
                                                                 audioFrame->nb_samples, out_sample_fmt, 1);

                emit sendAudioBuffer(audioBuffer, audioBufferSize);
                qDebug() << audioBuffer << audioBufferSize;
            }
        }
        av_free_packet(audioPacket);
    }

    av_frame_free(&audioFrame);
    av_free(audioBuffer);

    swr_free(&swrCtx);
    avcodec_close(audioCodecCtx);
}
