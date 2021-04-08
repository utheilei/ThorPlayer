#ifndef AUDIOHANDLE_H
#define AUDIOHANDLE_H

#include <QThread>
#include <QAudioOutput>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

class AudioHandle : public QThread
{
    Q_OBJECT
public:
    explicit AudioHandle(int id, AVFormatContext *avCodecContext, QObject *parent = nullptr);
    ~AudioHandle() override;

protected:
    void run() override;

signals:
    void sendAudioBuffer(const unsigned char *data, int datasize);

public slots:

private:
    AVFormatContext *m_formatCtx = Q_NULLPTR;
    int m_audioStream = -1;
    QAudioOutput *m_AudioOutput = Q_NULLPTR;
};

#endif // AUDIOHANDLE_H
