#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QThread>
#include <QImage>

class VideoPlayer : public QThread
{
    Q_OBJECT
public:
    explicit VideoPlayer();
    ~VideoPlayer() override;

    void setFileName(QString &name);

protected:
    void run() override;

signals:
    void sendBuffer(QSize size, const quint8 *dataY, const quint8 *dataU, const quint8 *dataV,
                    quint32 linesizeY, quint32 linesizeU, quint32 linesizeV);

    void sendAudioBuffer(const char *data, int size);

private:
    QList<QString> m_nameList;
    QString m_currentFile;
};

#endif // VIDEOPLAYER_H
