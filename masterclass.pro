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
    src/prism.cpp

HEADERS  += $$PWD/include/*.h \
    include/prism.h

FORMS    += $$PWD/forms/mainwindow.ui

DISTFILES += \
        shaders/vert.glsl \
        shaders/frag.glsl

#unix
#{
##Assimp libraries
#LIBS += -L$$PWD/libs/assimp/lib/ -lassimp

#Bullet libraries
#LIBS += -L$$PWD/libs/bullet/src -lBulletDynamics -lBulletCollision -lLinearMath
#}

#win32
#{
#Assimp libraries
LIBS += -L"E:\mattt\Documents\MPCLibraries\assimp-3.3.1\lib\Debug" -lassimp-vc140-mt

#Bullet Libraries
LIBS += -L"E:\mattt\Documents\MPCLibraries\bullet\build3\lib\Release" -lBulletDynamics \ # -lBulletDynamics \
			 -L"E:\mattt\Documents\MPCLibraries\bullet\build3\lib\Release" -lBulletCollision \# -lBulletCollision \
			 -L"E:\mattt\Documents\MPCLibraries\bullet\build3\lib\Release" -lLinearMath #-lLinearMath

LIBS += -lopengl32

INCLUDEPATH += $$PWD\libs\bullet\src\BulletDynamics\ \
							$$PWD\libs\bullet\src\BulletCollision\  \
							$$PWD\libs\bullet\src\LinearMath\

INCLUDEPATH += "E:\mattt\Documents\MPCLibraries\bullet/src/"
#}



#LIBS += -L$$PWD/libs/bullet/src -lBulletDynamics -lBulletCollision -lLinearMath

INCLUDEPATH += $$PWD/include/ \
				$$PWD/libs/bullet/src/ \
				$$PWD/libs/assimp/include/
