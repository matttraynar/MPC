#-------------------------------------------------
#
# Project created by QtCreator 2016-11-16T14:15:13
#
#-------------------------------------------------

QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET      = MPC
TEMPLATE    = app
MOC_DIR     = moc
OBJECTS_DIR = obj

SOURCES += $$PWD/src/*.cpp \

HEADERS  += $$PWD/include/*.h \

FORMS    += $$PWD/forms/mainwindow.ui

DISTFILES += \
        shaders/vert.glsl \
        shaders/frag.glsl

#Assimp libraries
LIBS += -L$$PWD/libs/assimp/lib/ -lassimp

#Bullet libraries
LIBS += -L$$PWD/libs/bullet/src -lBulletDynamics -lBulletCollision -lLinearMath

INCLUDEPATH += $$PWD/include/ \
               $$PWD/libs/bullet/src/
