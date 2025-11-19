#include "mainwindow.h"

#include <QApplication>
#include <QLockFile>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include "settings.h"
#include <QStyleFactory>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    QString lockPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(lockPath);

    QLockFile lockFile(lockPath + "/app.lock");
    lockFile.setStaleLockTime(0);

    if (!lockFile.tryLock()) {

#ifdef _WIN32
        HWND hwnd = FindWindowW(nullptr, L"App-Directory");
        if (hwnd) {
            ShowWindow(hwnd, SW_RESTORE);   
            SetForegroundWindow(hwnd);    
        }
#endif
        return 0;
    }

    a.setWindowIcon(QIcon(":/icons/icon.ico"));
    a.setStyle(QStyleFactory::create("Fusion"));
    Settings::loadSavedFont();
    Settings::loadSavedFontSize();

    MainWindow w;
    w.show();

    return a.exec();
}
