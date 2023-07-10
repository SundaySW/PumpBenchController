#include "mainwindow.h"

#include <QApplication>
#include "config.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.setWindowTitle(QString("%1 %2").arg(kPROJECT_NAME, kPROJECT_VER));
    w.show();
    return a.exec();
}
