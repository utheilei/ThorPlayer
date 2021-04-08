QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets multimedia

TARGET = ThorPlayer
TEMPLATE = app
CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget

SOURCES += \
        main.cpp \
    mainwindow.cpp \
    videorenderwidget.cpp \
    videoplayer.cpp \
    arrowrectangle.cpp \
    audiohandle.cpp

RESOURCES +=         resources.qrc

HEADERS += \
    mainwindow.h \
    videorenderwidget.h \
    videoplayer.h \
    arrowrectangle.h \
    audiohandle.h

unix|win32: LIBS += -L$$PWD/3rdparty/ -lavcodec
unix|win32: LIBS += -L$$PWD/3rdparty/ -lavformat
unix|win32: LIBS += -L$$PWD/3rdparty/ -lavutil
unix|win32: LIBS += -L$$PWD/3rdparty/ -lswscale
unix|win32: LIBS += -L$$PWD/3rdparty/ -lswresample

INCLUDEPATH += $$PWD/3rdparty
DEPENDPATH += $$PWD/3rdparty
