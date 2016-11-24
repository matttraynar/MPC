#include <QApplication>
#include <QDesktopWidget>

#include "glwidget.h"

int main( int argc, char* argv[] )
{
 QApplication a( argc, argv );

// Specify an OpenGL 3.3 format using the Core profile.
 // That is, no old-school fixed pipeline functionality

// Create a GLWidget requesting our format
 GLWidget w;
 w.setMinimumHeight(720);
 w.setMinimumWidth(900);

 w.move(QApplication::desktop()->screen()->rect().center() - w.rect().center());

 w.show();

return a.exec();
}
