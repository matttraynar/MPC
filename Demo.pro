#-------------------------------------------------
#
# Project created by QtCreator 2016-11-16T14:15:13
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Demo
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    glwidget.cpp

HEADERS  += mainwindow.h \
    glwidget.h

FORMS    += mainwindow.ui

DISTFILES += \
    shaders/vert.glsl \
    shaders/frag.glsl \
    demo.frag \
    demo.vert
