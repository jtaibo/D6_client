#include "mainwindow.h"
#include "d6.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    D6 d6;

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
