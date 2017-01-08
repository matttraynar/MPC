#include <QApplication>
#include <QDesktopWidget>

//#include "glwidget.h"
#include "mainwindow.h"

int main( int argc, char* argv[] )
{
 QApplication a( argc, argv );

// Create a GLWidget requesting our format
 MainWindow w;

 w.move(QApplication::desktop()->screen()->rect().center() - w.rect().center());

 w.show();

return a.exec();
}
