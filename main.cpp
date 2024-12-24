#include "mainwindow.h"

#include <QApplication>
#include "settings.h"
#include <QStyleFactory>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/icons/icon.ico"));
    a.setStyle(QStyleFactory::create("Fusion"));
    Settings::loadSavedFont();
    Settings::loadSavedFontSize();
    MainWindow w;
    w.show();
    return a.exec();
}
