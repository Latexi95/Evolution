#-------------------------------------------------
#
# Project created by QtCreator 2015-01-30T07:43:56
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Evolution
TEMPLATE = app

CONFIG += c++11


QMAKE_CXXFLAGS_RELEASE = -O3

QMAKE_CXXFLAGS_RELEASE += -g
QMAKE_CFLAGS_RELEASE += -g
QMAKE_LFLAGS_RELEASE =

CONFIG(debug, debug|release) {
    DEFINES += DEBUG
}

SOURCES += main.cpp\
        mainwindow.cpp \
    mapviewwidget.cpp \
    entity.cpp \
    entityproperty.cpp \
    position.cpp \
    map.cpp \
    action.cpp \
    entityupdatetask.cpp \
    bytecodedialog.cpp \
    worker.cpp

HEADERS  += mainwindow.h \
    mapviewwidget.h \
    entity.h \
    entityproperty.h \
    position.h \
    map.h \
    enums.h \
    action.h \
    entityupdatetask.h \
    bytecodedialog.h \
    worker.h

FORMS    += mainwindow.ui \
    bytecodedialog.ui
