#Specify the Qt type
QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

#Specify where to put the various files during make/build
TARGET      = MPC
TEMPLATE    = app
MOC_DIR     = moc
OBJECTS_DIR = obj

#Include all source files
SOURCES += $$PWD/src/*.cpp

#Include all header files
HEADERS  += $$PWD/include/*.h

#Specify where the UI form is
FORMS    += $$PWD/forms/mainwindow.ui

#Include the shaders
DISTFILES += \
        shaders/vert.glsl \
        shaders/frag.glsl

#Unix paths
#Assimp library
unix::LIBS += -L$$PWD/libs/assimp/lib/ -lassimp

#Bullet libraries
unix::LIBS += -L$$PWD/libs/bullet/src -lBulletDynamics -lBulletCollision -lLinearMath


#Windows paths
#Assimp library
win32::LIBS += -L"E:\mattt\Documents\MPCLibraries\assimp-3.3.1\lib\Debug" -lassimp-vc140-mt

#Bullet Libraries
win32::LIBS += -L"E:\mattt\Documents\MPCLibraries\bullet\build3\lib\Release" -lBulletDynamics \
			 -L"E:\mattt\Documents\MPCLibraries\bullet\build3\lib\Release" -lBulletCollision \
			 -L"E:\mattt\Documents\MPCLibraries\bullet\build3\lib\Release" -lLinearMath

#Windows explicitly needs OpenGL
win32::LIBS += -lopengl32

#Include the bullet source file paths we need
win32::INCLUDEPATH += $$PWD\libs\bullet\src\BulletDynamics\ \
							$$PWD\libs\bullet\src\BulletCollision\  \
							$$PWD\libs\bullet\src\LinearMath

#Final include paths for other files
INCLUDEPATH += $$PWD/include/ \
				$$PWD/libs/bullet/src/ \
				$$PWD/libs/assimp/include/
