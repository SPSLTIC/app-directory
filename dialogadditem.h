#ifndef DIALOGADDITEM_H
#define DIALOGADDITEM_H

#include <QDialog>
#include "config.h"
#include "richitem.h"
#include <QJsonObject>
#include <QDir>

namespace Ui {
class DialogAddItem;
}

class DialogAddItem : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddItem(QWidget *parent = nullptr);
    explicit DialogAddItem(const QJsonObject& entry, QWidget* parent = nullptr);
    ~DialogAddItem();
    QJsonObject getNewEntry() const { return newEntry; }

signals:
    void customEntryCreated(const QJsonObject &entry);
    void customEntryUpdate(const QJsonObject &entry);
    void customEntryDeleted(int id);

private slots:
    void generateEntry();
    void on_pushButton_2_pressed();
    void on_pushButton_3_pressed();

private:
    Ui::DialogAddItem *ui;
    bool verifyEntry();
    QJsonObject newEntry;
    int generateUniqueId();
    QString getCustomEntriesPath() const {
        return QDir(Config::CUSTOM_DATA_PATH).filePath(Config::CUSTOM_FILENAME);
    }
};

#endif // DIALOGADDITEM_H
