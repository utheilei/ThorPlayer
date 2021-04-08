#include "arrowrectangle.h"

#include <QVBoxLayout>
#include <QLabel>
#include <DIconButton>
#include <DApplicationHelper>

ArrowRectangle::ArrowRectangle(DArrowRectangle::ArrowDirection direction, QWidget *parent)
    : DArrowRectangle(direction, FloatWidget, parent)
{
    setFixedSize(QSize(65, 205));
    setShadowBlurRadius(4);
    setRadius(18);
    setShadowYOffset(3);
    setShadowXOffset(0);
    setArrowWidth(20);
    setArrowHeight(15);
    setFocusPolicy(Qt::NoFocus);

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(2, 8, 2, 2);
    widget->setLayout(layout);

    m_label = new QLabel(this);
    m_label->setText("1%");
    m_slider = new DSlider(Qt::Vertical, this);
    m_slider->setMinimum(0);
    m_slider->setMaximum(100);

    DIconButton *iconButton = new DIconButton(this);
    iconButton->setIcon(QIcon::fromTheme(":/images/volume.svg"));
    iconButton->setFlat(true);

    layout->addWidget(m_label);
    layout->addWidget(m_slider);
    layout->addWidget(iconButton);
    setContent(widget);

    initConnect();
}

void ArrowRectangle::initConnect()
{
    connect(m_slider, &DSlider::valueChanged, this, &ArrowRectangle::onValueChanged);
}

int ArrowRectangle::currentVolume()
{
    return m_slider->value();
}

void ArrowRectangle::onValueChanged(int value)
{
    emit volumeChanged(value);
    m_label->setText(QString("%1%").arg(value));
}
