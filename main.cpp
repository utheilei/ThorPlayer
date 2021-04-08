#include "mainwindow.h"

#include <DApplication>
#include <DWidgetUtil>

DWIDGET_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.loadTranslator();
    app.setOrganizationName("deepin");
    app.setApplicationName("ThorPlayer");
    app.setApplicationVersion("1.0.0");
    app.setProductIcon(QIcon(":/images/hl_player_48px.svg"));
    app.setProductName("ThorPlayer");
    app.setApplicationDescription("This is a simple movie player.");

    MainWindow w;
    w.show();

    Dtk::Widget::moveToCenter(&w);

    return app.exec();
}
