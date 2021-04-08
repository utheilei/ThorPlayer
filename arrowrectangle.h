#ifndef ARROWRECTANGLE_H
#define ARROWRECTANGLE_H

#include <DArrowRectangle>
#include <DSlider>

DWIDGET_USE_NAMESPACE

class ArrowRectangle : public DArrowRectangle
{
    Q_OBJECT
public:
    explicit ArrowRectangle(ArrowDirection direction, QWidget *parent = nullptr);

    void initConnect();

    int currentVolume();

signals:
    void volumeChanged(int value);

public slots:
    void onValueChanged(int value);

private:
    QLabel *m_label = Q_NULLPTR;
    DSlider *m_slider = Q_NULLPTR;
};

#endif // ARROWRECTANGLE_H
