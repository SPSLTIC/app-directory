#include "dialogadditem.h"
#include "ui_dialogadditem.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFileInfo>
#include <QDir>

DialogAddItem::DialogAddItem(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogAddItem)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (verifyEntry()) {  // Only proceed if verification passes
            emit customEntryCreated(newEntry);
            accept();
        }
    });
}

DialogAddItem::~DialogAddItem()
{
    delete ui;
}

void DialogAddItem::generateEntry()
{
    accept();
    QMessageBox::information(this, "working", "Added");
}

bool DialogAddItem::verifyEntry()
{
    if(!ui->lineEdit->text().isEmpty() && !ui->lineEdit_2->text().isEmpty() && !ui->lineEdit_3->text().isEmpty())
    {
        newEntry = QJsonObject({
            {"ID", generateUniqueId()},
            {"Name", ui->lineEdit->text()},
            {"Path", ui->lineEdit_2->text()},
            {"Image", ui->lineEdit_3->text()},
            {"Favorite", true}
        });

        return true;
    } else {
        QMessageBox::warning(this, tr("Error"),
                             tr("Please fill in all fields"));
        return false;
    }
}

void DialogAddItem::on_pushButton_2_pressed()
{
    QFileDialog dialog(this);
    dialog.setDirectory(QDir::homePath());
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setWindowTitle("Sélectionner le programme");

    if (dialog.exec() == QDialog::Accepted)
    {
        QString fileName = dialog.selectedFiles().first();
        if (!fileName.isEmpty())
            ui->lineEdit_2->setText(fileName);
    }
}


void DialogAddItem::on_pushButton_3_pressed()
{
    QFileDialog dialog(this);
    dialog.setDirectory(QDir::homePath());
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setWindowTitle("Sélectionnez l'image");

    QStringList filters;
    filters << "Images (*.png *.jpg *.jpeg)"
            << "PNG Images (*.png)"
            << "JPEG Images (*.jpg *.jpeg)";
    dialog.setNameFilters(filters);

    if (dialog.exec() == QDialog::Accepted)
    {
        QString fileName = dialog.selectedFiles().first();
        if (!fileName.isEmpty())
        {
            QString fileName = dialog.selectedFiles().first();
            ui->lineEdit_3->setText(fileName);
            qDebug() << "MANUAL: Set image path to:" << fileName;
        }
    }
}

int DialogAddItem::generateUniqueId()
{
    QString customFilePath = getCustomEntriesPath();
    QFile userfilecustom(customFilePath);
    QJsonArray customEntries;

    if (userfilecustom.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(userfilecustom.readAll());
        userfilecustom.close();
        customEntries = doc.array();
        qDebug() << "Loaded" << customEntries.size() << "custom entries from" << customFilePath;
    } else {
        qDebug() << "No custom entries file found or couldn't open it:" << customFilePath;
        qDebug() << "Error:" << userfilecustom.errorString();
    }

    qDebug() << "Current custom entries:";
    for (const QJsonValue &value : customEntries) {
        QJsonObject obj = value.toObject();
        qDebug() << "  ID:" << obj["ID"].toInt() << "Name:" << obj["Name"].toString();
    }

    int newId = 9000;
    bool idExists;

    do {
        idExists = false;
        for (const QJsonValue &value : customEntries) {
            QJsonObject obj = value.toObject();
            if (obj["ID"].toInt() == newId) {
                qDebug() << "ID collision found:" << newId << "for entry:" << obj["Name"].toString();
                idExists = true;
                newId++;
                break;
            }
        }
    } while (idExists);

    return newId;
}
