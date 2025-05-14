#include "dialogadditem.h"
#include "ui_dialogadditem.h"
#include "mainwindow.h"
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
    ui->deleteButton->setVisible(false);

    prepareInterface();

    QPushButton* okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (okButton) {
        okButton->setText(tr("Ajouter"));
    }

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (verifyEntry()) {  // Only proceed if verification passes
            emit customEntryCreated(newEntry);
            accept();
        }
        });

    connect(ui->comboBox_type, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &DialogAddItem::onTypeChanged);

    onTypeChanged(ui->comboBox_type->currentIndex());
}

// Update the entry
DialogAddItem::DialogAddItem(const QJsonObject& entry, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DialogAddItem)
    , newEntry(entry)
{
    ui->setupUi(this);

    this->setWindowTitle("Modifier le raccourci");
    //TODO

    int typeNum;
    if (entry.value("Type").toString().isEmpty()) {
        typeNum = entry.value("Type").toInt();
    } else {
        typeNum = MainWindow().getTypeNum(entry.value("Type").toString());
    }

    onTypeChanged(typeNum);

    ui->lineEdit->setText(entry.value("Name").toString());
    ui->lineEdit_2->setText(entry.value("Path").toString());
    ui->lineEdit_3->setText(entry.value("Image").toString());

    previewIcon(entry.value("ID").toInt());

    prepareInterface();

    QPushButton* okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (okButton) {
        okButton->setText(tr("Sauvegarder"));
    }

    ui->deleteButton->setStyleSheet("background-color: #C91100; color: white;");

    ui->deleteButton->show();
    //TODO
    ui->comboBox_type->setCurrentIndex(typeNum);
  

    connect(ui->deleteButton, &QPushButton::clicked, this, [this]() {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle(tr("Supprimer le raccourci"));
        msgBox.setText(tr("Êtes-vous sûr de vouloir supprimer ce raccourci ?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setButtonText(QMessageBox::Yes, tr("Oui"));
        msgBox.setButtonText(QMessageBox::No, tr("Non"));

        int ret = msgBox.exec();
        if (ret == QMessageBox::Yes) {
            emit customEntryDeleted(newEntry["ID"].toInt());
            reject();
        }
        });

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, [this]() {
        if (verifyEntry()) {
            qDebug() << "Signal customEntryUpdate émis avec:" << newEntry;
            emit customEntryUpdate(newEntry);
            accept();
        }
        });

    connect(ui->comboBox_type, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &DialogAddItem::onTypeChanged);
}

void DialogAddItem::prepareInterface() {

    QPushButton* cancelButton = ui->buttonBox->button(QDialogButtonBox::Cancel);
    if (cancelButton) {
        cancelButton->setText(tr("Annuler"));
    }

    ui->label->setTextFormat(Qt::RichText);
    ui->label->setText("*Nom de l'application <br> <i>Champ requis</i>");

    ui->label_4->setTextFormat(Qt::RichText);
    ui->label_4->setText("*Type d'application <br> <i>Champ requis</i>");

    ui->comboBox_type->addItem("Site internet");
    ui->comboBox_type->addItem("Autre");
    ui->lineEdit->setPlaceholderText("Par ex: Google");

    // Connect signals for each input field
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &DialogAddItem::checkFields);
    connect(ui->lineEdit_2, &QLineEdit::textChanged, this, &DialogAddItem::checkFields);
    connect(ui->lineEdit_3, &QLineEdit::textChanged, this, &DialogAddItem::checkFields);

    // Call it initially to set the proper state
    checkFields();
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
    if (!ui->lineEdit->text().isEmpty() &&
        !ui->lineEdit_2->text().isEmpty())
    {
        int id = newEntry.contains("ID") ? newEntry.value("ID").toInt() : -1;

        bool favorite = newEntry.contains("Favorite") ? newEntry.value("Favorite").toBool() : true;

        if (ui->lineEdit_3->text().isEmpty()) {
            if (ui->comboBox_type->currentIndex() == 0) { //web
                ui->lineEdit_3->setText(":/icons/icons/web.png");
            }
            else {
                ui->lineEdit_3->setText(":/icons/icons/other.png");
            }
        }


        QString typeName;

        if (ui->comboBox_type->currentIndex() == 0) {
            typeName = "Web";
        }
        else if (ui->comboBox_type->currentIndex() == 1) {
            typeName = "Autre";
        }

        newEntry = QJsonObject({
            {"ID", id},
            {"Name", ui->lineEdit->text()},
            {"Path", ui->lineEdit_2->text()},
            {"Image", ui->lineEdit_3->text()},
            {"Favorite", favorite},
            {"Custom", true},
            {"Type", typeName}
            });

        return true;
    }
    else
    {
        QMessageBox::warning(this, tr("Erreur"),
            tr("Veuillez remplir tous les champs"));
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

    previewIcon(-1);
}

void DialogAddItem::onTypeChanged(int index)
{
    switch (index)
    {
    case 0: // web
        ui->pushButton_2->setVisible(false);
        ui->lineEdit_2->setEnabled(true);
        ui->lineEdit_2->setPlaceholderText("https://google.ch");
        ui->lineEdit_2->setText("");
        break;
    case 1: // other
        ui->pushButton_2->setVisible(true);
        ui->lineEdit_2->setEnabled(false);
        ui->lineEdit_2->setPlaceholderText("");
        ui->lineEdit_2->setText("");
        break;
    default:
        ui->pushButton_2->setVisible(false);
        ui->lineEdit_2->setEnabled(false);
        break;
    }

    previewIcon(index);
}

void DialogAddItem::previewIcon(int id) {
    qDebug() << "id" << id;
    QPixmap pixmap;

    if (ui->lineEdit_3->text().isEmpty()) {
        if (id == 0) {
            pixmap = QPixmap(":/icons/icons/web.png");
        }
        else if (id == 1) {
            pixmap = QPixmap(":/icons/icons/other.png");
        }
        else {
            pixmap = ui->label_2->pixmap();
        }
        
    }
    else {
        pixmap = QPixmap(ui->lineEdit_3->text());
    }


    if (!pixmap.isNull()) {
        qDebug() << "isNotNull";
        pixmap = pixmap.scaled(45, 45, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        ui->label_2->setPixmap(pixmap);
    }
    else {
        qDebug() << "isNull";
    }
}


int DialogAddItem::generateUniqueId(const QJsonArray& jsonArray)
{
    QJsonArray customEntries = jsonArray;

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

void DialogAddItem::checkFields()
{
    // Check if all required fields are not empty (you can adjust conditions as needed)
    bool allFilled = !ui->lineEdit->text().trimmed().isEmpty() &&
        !ui->lineEdit_2->text().trimmed().isEmpty();

    // Enable or disable the OK/Add button accordingly
    QPushButton* okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (okButton) {
        okButton->setEnabled(allFilled);
    }
}
