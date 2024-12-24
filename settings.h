#ifndef SETTINGS_H
#define SETTINGS_H

#include "config.h"
#include <QDialog>
#include <QSettings>
#include <QDir>
#include <QApplication>

namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();
    static void loadSavedFontSize();
    static void loadSavedFont();

private:
    Ui::Settings *ui;

    enum FontSize {
        SMALL = 11,
        MEDIUM = 12,
        LARGE = 14
    };

    void initializeFont();
    void initializeFontSize();
    void initializeStartupSetting();
    QSettings guiSettings{"TIC", "AppDirectory"};
    void applyFont(const QString &fontFamily);
    void applyFontSize(FontSize size);
    void updateStartupSetting(bool enabled);
    QStringList m_allowedFonts;
    void setupFontPicker();

private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_checkBox_stateChanged(int arg1);
};

#endif // SETTINGS_H
