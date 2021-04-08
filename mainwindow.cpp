#include "mainwindow.h"
#include "videorenderwidget.h"
#include "videoplayer.h"
#include "arrowrectangle.h"

#include <DApplication>
#include <DMenu>
#include <DButtonBox>

#include <QTime>
#include <QDebug>
#include <QHBoxLayout>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
{
    m_titlebar = this->titlebar();
    m_titlebar->setDisableFlags(Qt::WindowMinimizeButtonHint
                                | Qt::WindowMaximizeButtonHint
                                | Qt::WindowSystemMenuHint);
    m_titlebar->setAutoHideOnFullscreen(true);
    m_titlebar->setIcon(qApp->productIcon());
    m_titlebar->setTitle(qApp->applicationName());

    m_arrowRectangle = new ArrowRectangle(DArrowRectangle::ArrowBottom, this);

    DMenu *pDMenu = new DMenu;
    QAction *action(new QAction(tr("Help"), this));
    QAction *openAction(new QAction(tr("Open"), this));
    pDMenu->addAction(openAction);
    pDMenu->addAction(action);
    m_titlebar->setMenu(pDMenu);
    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);

    m_videoRenderWidget = new VideoRenderWidget(this);
    setCentralWidget(m_videoRenderWidget);

    initFloatWidget();
    initContent();
    initConnection();
}

MainWindow::~MainWindow()
{
    if (mPlayer->isRunning()) {
        mPlayer->quit();
        mPlayer->deleteLater();
    }
}

void MainWindow::initContent()
{
    mPlayer = new VideoPlayer;
}

void MainWindow::initFloatWidget()
{
    m_floatingWidget = new DFloatingWidget(this);
    QWidget *widget = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout;
    widget->setLayout(layout);

    m_buttonBox = new DButtonBox(this);
    m_buttonBox->setFixedHeight(50);
    DButtonBoxButton *preButton = new DButtonBoxButton(QIcon::fromTheme(":/images/before.svg"), "", this);
    preButton->setIconSize(QSize(15, 15));
    DButtonBoxButton *playButton = new DButtonBoxButton(QIcon::fromTheme(":/images/play.svg"), "", this);
    playButton->setIconSize(QSize(15, 15));
    DButtonBoxButton *nextButton = new DButtonBoxButton(QIcon::fromTheme(":/images/next.svg"), "", this);
    nextButton->setIconSize(QSize(15, 15));
    QList<DButtonBoxButton*> list = {preButton, playButton, nextButton};
    m_buttonBox->setButtonList(list, false);

    QLabel *label = new QLabel("00:00:00", this);
    m_slider = new DSlider(Qt::Horizontal, this);
    QLabel *totalLabel = new QLabel("00:00:00", this);

    DIconButton *fullButton = new DIconButton(this);
    fullButton->setFixedSize(50, 50);
    fullButton->setIcon(QIcon::fromTheme(":/images/fullsreen.svg"));
    fullButton->setIconSize(QSize(20, 20));
    connect(fullButton, &DIconButton::clicked, [=](){
        if(this->isFullScreen()) {
            this->showNormal();
            fullButton->setIcon(QIcon::fromTheme(":/images/fullsreen.svg"));
        } else {
            this->showFullScreen();
            fullButton->setIcon(QIcon::fromTheme(":/images/quit.svg"));
        }
    });

    m_volumeButton = new DIconButton(this);
    m_volumeButton->setFixedSize(50, 50);
    m_volumeButton->setIcon(QIcon::fromTheme(":/images/volume.svg"));
    m_volumeButton->setIconSize(QSize(20, 20));

    DIconButton *listButton = new DIconButton(this);
    listButton->setFixedSize(50, 50);
    listButton->setIcon(QIcon::fromTheme(":/images/list.svg"));
    listButton->setIconSize(QSize(20, 20));

    layout->addWidget(m_buttonBox);
    layout->addWidget(label);
    layout->addWidget(m_slider);
    layout->addWidget(totalLabel);
    layout->addWidget(fullButton);
    layout->addWidget(m_volumeButton);
    layout->addWidget(listButton);

    m_floatingWidget->setWidget(widget);
}

void MainWindow::initConnection()
{
    connect(m_volumeButton, &DIconButton::clicked, [=](){
        m_arrowRectangle->setVisible(true);
        m_arrowRectangle->raise();
        QPoint point = m_floatingWidget->geometry().topLeft() + m_volumeButton->geometry().topLeft();
        m_arrowRectangle->move(point.x() + 30, point.y() - 8);
        QTimer::singleShot(3000, m_arrowRectangle, &ArrowRectangle::hide);
    });

    connect(mPlayer, &VideoPlayer::sendBuffer, m_videoRenderWidget, &VideoRenderWidget::updateBuffer, Qt::QueuedConnection);
    connect(mPlayer, &VideoPlayer::sendAudioBuffer, this, &MainWindow::playAudio, Qt::QueuedConnection);
}

QString MainWindow::formatTime(int s)
{
    int hour = s / 3600;
    int minute = s / 60;
    int second = s % 60;

    QTime time(hour, minute, second);

    return time.toString("hh:mm:ss");
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    m_floatingWidget->setFixedSize(this->width() - 20, 80);
    m_floatingWidget->move(10, this->height() - 90);
    m_arrowRectangle->setVisible(false);

    DMainWindow::resizeEvent(event);
}

void MainWindow::leaveEvent(QEvent *event)
{
    m_floatingWidget->setVisible(false);
    DMainWindow::leaveEvent(event);
}

void MainWindow::enterEvent(QEvent *event)
{
    m_floatingWidget->setVisible(true);
    DMainWindow::enterEvent(event);
}

void MainWindow::openFile()
{
    QString tempPath = QFileDialog::getOpenFileName(this, tr("open file"), "../", "*.*");

    if(!tempPath.isEmpty()) {
        mPlayer->setFileName(tempPath);
        mPlayer->start();
    }
}

void MainWindow::playAudio(const char *data, int size)
{
    qDebug() << QString::fromLocal8Bit(data) << size;
    if (m_AudioOutput == Q_NULLPTR) {
        QAudioFormat fmt; //Qt音频的格式
        fmt.setSampleRate(44100); //采样率
        fmt.setSampleSize(16); //位数
        fmt.setChannelCount(3);  //声道
        fmt.setCodec("audio/pcm"); //音频的格式
        fmt.setByteOrder(QAudioFormat::LittleEndian); //次序
        fmt.setSampleType(QAudioFormat::SignedInt); //样本的类别

        QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
        if (!info.isFormatSupported(fmt)) {
            qDebug() << "Raw audio format not supported by backend, cannot play audio.";
            return;
        }

        m_AudioOutput = new QAudioOutput(fmt, this);
        m_ioDevice = m_AudioOutput->start();
    }

    if (m_ioDevice && !QString::fromLocal8Bit(data).isEmpty())
        qDebug() << m_ioDevice->write(QString::fromLocal8Bit(data).toUtf8(), size);
}
