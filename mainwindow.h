#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DMainWindow>
#include <DTitlebar>
#include <DFloatingWidget>
#include <DSlider>
#include <DButtonBox>

#include <QAudioOutput>

DWIDGET_USE_NAMESPACE

class VideoRenderWidget;
class VideoPlayer;
class ArrowRectangle;
class MainWindow : public DMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void initContent();

    void initFloatWidget();

    void initConnection();

private:
    QString formatTime(int s);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void enterEvent(QEvent *event) override;

signals:

public slots:
    void openFile();
    void playAudio(const char *data, int size);

private:
    DTitlebar *m_titlebar = Q_NULLPTR;
    VideoRenderWidget *m_videoRenderWidget = Q_NULLPTR;
    VideoPlayer *mPlayer = Q_NULLPTR;
    DFloatingWidget *m_floatingWidget = Q_NULLPTR;
    DSlider *m_slider = Q_NULLPTR;
    DButtonBox *m_buttonBox = Q_NULLPTR;
    ArrowRectangle *m_arrowRectangle = Q_NULLPTR;
    DIconButton *m_volumeButton = Q_NULLPTR;
    QAudioOutput *m_AudioOutput = Q_NULLPTR;
    QIODevice *m_ioDevice = Q_NULLPTR;
};

#endif // MAINWINDOW_H
