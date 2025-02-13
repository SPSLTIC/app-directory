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

    this->setWindowTitle("Modifier le raccourcis");

    prepareInterface();

    QPushButton* okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (okButton) {
        okButton->setText(tr("Sauvegarder"));
    }

    ui->deleteButton->setStyleSheet("background-color: #C91100; color: white;");

    onTypeChanged(entry.value("Type").toInt());

    ui->lineEdit->setText(entry.value("Name").toString());
    ui->lineEdit_2->setText(entry.value("Path").toString());
    ui->lineEdit_3->setText(entry.value("Image").toString());

    ui->deleteButton->show();

    ui->comboBox_type->setCurrentIndex(entry.value("Type").toInt());
  

    connect(ui->deleteButton, &QPushButton::clicked, this, [this]() {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Question);
        msgBox.setWindowTitle(tr("Supprimer le raccourcis"));
        msgBox.setText(tr("Êtes-vous sûr de vouloir supprimer ce raccourcis ?"));
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
        !ui->lineEdit_2->text().isEmpty() &&
        !ui->lineEdit_3->text().isEmpty())
    {
        int id = newEntry.contains("ID") ? newEntry.value("ID").toInt() : generateUniqueId();

        bool favorite = newEntry.contains("Favorite") ? newEntry.value("Favorite").toBool() : true;

        newEntry = QJsonObject({
            {"ID", id},
            {"Name", ui->lineEdit->text()},
            {"Path", ui->lineEdit_2->text()},
            {"Image", ui->lineEdit_3->text()},
            {"Favorite", favorite},
            {"Custom", true},
            {"Type", ui->comboBox_type->currentIndex()}
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

void DialogAddItem::checkFields()
{
    // Check if all required fields are not empty (you can adjust conditions as needed)
    bool allFilled = !ui->lineEdit->text().trimmed().isEmpty() &&
        !ui->lineEdit_2->text().trimmed().isEmpty() &&
        !ui->lineEdit_3->text().trimmed().isEmpty();

    // Enable or disable the OK/Add button accordingly
    QPushButton* okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (okButton) {
        okButton->setEnabled(allFilled);
    }
}
