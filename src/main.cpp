#include <QApplication>
#include <QDesktopWidget>

#include "mainwindow.h"

int main( int argc, char* argv[] )
{
    //Create a new application
    QApplication a( argc, argv );

    // Create a window
    MainWindow w;

    //Center the window on the user's screen
    w.move(QApplication::desktop()->screen()->rect().center() - w.rect().center());

    //Show the window
    w.show();

    return a.exec();
}
