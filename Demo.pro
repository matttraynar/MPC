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
    glwidget.cpp \
    modelloader.cpp \
    assimploader.cpp

HEADERS  += mainwindow.h \
    glwidget.h \
    modelloader.h \
    assimploader.h

FORMS    += mainwindow.ui

DISTFILES += \
    shaders/vert.glsl \
    shaders/frag.glsl

LIBS += -L$$PWD/libs/assimp/lib/ -lassimp

INCLUDEPATH += $$PWD/include/
