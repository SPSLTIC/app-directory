#include "settings.h"
#include "ui_settings.h"
#include <QSettings>
#include <QPalette>
#include <QDir>
#include <QFontDatabase>
#include <QProxyStyle>
#include "config.h"

Settings::Settings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Settings)
{
    ui->setupUi(this);
    setupFontPicker();
    initializeFont();
    initializeFontSize();
    initializeStartupSetting();
}

Settings::~Settings()
{
    delete ui;
}

/*
 * everything is separated because I wanted to try it out this
 * way you can definitely centralize the logic a bit more.
 */

void Settings::loadSavedFont() {
    QSettings guiSettings("TIC", "AppDirectory");
    QString systemFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont).family();
    QString savedFont = guiSettings.value("fontFamily", systemFont).toString();
    QFont font = QApplication::font();
    font.setFamily(savedFont);
    QApplication::setFont(font);
}

void Settings::loadSavedFontSize()
{
    QSettings guiSettings("TIC", "AppDirectory");
    int savedSize = guiSettings.value("fontSize", MEDIUM).toInt();
    QFont font = QApplication::font();
    font.setPointSize(savedSize);
    QApplication::setFont(font);
}

void Settings::initializeFont() {
    QSettings guiSettings("TIC", "AppDirectory");
    QString currentFont = guiSettings.value("fontFamily",
                                            QFontDatabase::systemFont(QFontDatabase::GeneralFont).
                                            family()).toString();

    QString customFont;
    if (Config::HAS_CUSTOM_FONT) {
        customFont = QString(Config::SPECIFIC_FONT).split("Regular.ttf").first();
    }

    ui->comboBox_2->clear();
    // simple hack to have a space in the ComboBox (can probably fixed in the .ui file)
    ui->comboBox_2->addItem(" Police du système");
    if (!customFont.isEmpty()) {
        ui->comboBox_2->addItem(" " + customFont);
    }

    // set current based on saved setting
    if (currentFont == customFont) {
        ui->comboBox_2->setCurrentIndex(1);
    } else {
        ui->comboBox_2->setCurrentIndex(0);
    }
}

void Settings::initializeFontSize()
{
    QSettings guiSettings("TIC", "AppDirectory");
    int currentSize = guiSettings.value("fontSize", MEDIUM).toInt();

    // update button states based on current size
    ui->pushButton->setChecked(currentSize == SMALL);
    ui->pushButton_2->setChecked(currentSize == MEDIUM);
    ui->pushButton_3->setChecked(currentSize == LARGE);
}

void Settings::applyFont(const QString &fontFamily) {
    QFont font = QApplication::font();
    font.setFamily(fontFamily);
    QApplication::setFont(font);

    QSettings guiSettings("TIC", "AppDirectory");
    guiSettings.setValue("fontFamily", fontFamily);
    guiSettings.sync();
}

void Settings::applyFontSize(FontSize size)
{
    QFont font = QApplication::font();
    font.setPointSize(size);
    QApplication::setFont(font);

    guiSettings.setValue("fontSize", size);
    guiSettings.sync();
}

void Settings::on_pushButton_clicked()
{
    applyFontSize(SMALL);
}

void Settings::on_pushButton_2_clicked()
{
    applyFontSize(MEDIUM);
}

void Settings::on_pushButton_3_clicked()
{
    applyFontSize(LARGE);
}

void Settings::initializeStartupSetting()
{
    QSettings guiSettings("TIC", "AppDirectory");
    bool startupEnabled = guiSettings.value("startOnBoot", false).toBool();
    ui->checkBox->setChecked(startupEnabled);
}

void Settings::updateStartupSetting(bool enabled)
{
    QSettings guiSettings("TIC", "AppDirectory");
    guiSettings.setValue("startOnBoot", enabled);

    QSettings bootSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                           QSettings::NativeFormat);

    if (enabled) {
        QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
        bootSettings.setValue("AppDirectory", appPath);
    } else {
        bootSettings.remove("AppDirectory");
    }
}

void Settings::on_checkBox_stateChanged(int arg1)
{
    bool enabled = (arg1 == Qt::Checked);
    updateStartupSetting(enabled);
}

void Settings::setupFontPicker()
{
    initializeFont();

    connect(ui->comboBox_2, &QComboBox::currentTextChanged,
        this, [this](const QString& fontFamily) {
            QString selectedFont = fontFamily.trimmed();
            if (selectedFont == "Police du système") {
                selectedFont = QFontDatabase::systemFont(QFontDatabase::GeneralFont).family();
            } else if (Config::HAS_CUSTOM_FONT) {
                selectedFont = QString(Config::SPECIFIC_FONT).split("Regular.ttf").first();
            }
            applyFont(selectedFont);
        }
    );
}
