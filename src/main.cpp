#include <QApplication>

#include "glwidget.h"

int main( int argc, char* argv[] )
{
 QApplication a( argc, argv );

// Specify an OpenGL 3.3 format using the Core profile.
 // That is, no old-school fixed pipeline functionality

// Create a GLWidget requesting our format
 GLWidget w;
 w.show();

return a.exec();
}
