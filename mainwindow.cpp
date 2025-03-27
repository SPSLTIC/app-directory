#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "richitem.h"
#include "dialogadditem.h"
#include "settings.h"
#include "propos.h"
#include "dialogaddfavorites.h"
#include <QListWidget>
#include <QPushButton>
#include <QComboBox>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDialog>
#include <QCloseEvent>
#include <QSaveFile>
#include <QSettings>
#include <QToolButton>
#include <QSizeGrip>
#include <QDate>
#include <QProcess>
#include <QEventLoop>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSettings settings("AppDirectory", "MainWindow");
    restoreGeometry(settings.value("MainWindowGeometry").toByteArray());
    restoreState(settings.value("MainWindowState").toByteArray());


    QString sourceDir = Config::DEFAULT_PATH_SRC;
    QString destDir = Config::DEFAULT_JSON_PATH;

    if (copyDirectoryWithRobocopy(sourceDir, destDir)) {
        qDebug() << "Copie effectuée avec succès.";
    }
    else {
        qDebug() << "La copie a échoué ou n'a pas été nécessaire.";
    }


    /*
     * the cornerWidget is set by taking an existing widget (the comboBox + the pushButton_3)
     * it needs to be a single widget because it can't add two, it also can't be in tabWidget first
     */
    ui->tabWidget->setCornerWidget(ui->cornerComboWidget, Qt::TopRightCorner);
    ui->lineEdit->setClearButtonEnabled(true);
    ui->listWidget_2->setDragEnabled(true);
    ui->listWidget_2->setAcceptDrops(true);
    ui->listWidget_2->setDropIndicatorShown(true);
    ui->listWidget_2->setDragDropMode(QAbstractItemView::InternalMove);
        
    ui->listWidget->setCursor(Qt::PointingHandCursor);
    ui->listWidget_2->setCursor(Qt::PointingHandCursor);
    connect(ui->listWidget_2->model(), &QAbstractItemModel::rowsMoved,
            this, &MainWindow::updateFavoriteOrder);

    setupSortCriteria();

    setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);
    
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);

    currentSortRole = SortRole::Favorite;
    currentSortOrder = Qt::AscendingOrder;

    ui->comboBox->hide();
   
    if (!loadData()) {
      QMessageBox::warning(this, tr("Erreur"),
                             tr("Certaines données n'ont pas pu être chargées"));  
      qApp->quit();
    } else {
        /*
         * if the data is loaded it means it has the default info for the favorites
         * so it should show the favorite tab (tab_2)
         */

        if (userJsonArray.size() > 0) {
            ui->tabWidget->setCurrentIndex(1);
            onTabChanged(1);
        }
        else {
            ui->tabWidget->setCurrentIndex(0);
            onTabChanged(0);
        }
    }

    QSettings bootSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
        QSettings::NativeFormat);

    QSettings config("HKEY_CURRENT_USER\\Software\\AppDirectory", QSettings::NativeFormat);

    // Check if it's the first run
    if (!config.contains("AppDirectory") && !bootSettings.contains("AppDirectory")) {
       QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
       bootSettings.setValue("AppDirectory", appPath);
    }

    
    populateList();

    connect(ui->pushButton_3, &QPushButton::clicked,
            this, &MainWindow::onSortOrderChanged);

    connect(ui->tabWidget, &QTabWidget::currentChanged,
        this, &MainWindow::onTabChanged);
}

bool MainWindow::checkInternetConnection()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Problème de connexion au réseau de la Ville");
    msgBox.setText("Assurez-vous d'être connecté au réseau de la Ville avant de réessayer.");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Ok);

    msgBox.show();
    QApplication::processEvents();

    QPushButton* retryButton = qobject_cast<QPushButton*>(msgBox.button(QMessageBox::Ok));
    if (retryButton)
        retryButton->setText("Réessayer");

    QPushButton* quitButton = qobject_cast<QPushButton*>(msgBox.button(QMessageBox::No));
    if (quitButton)
        quitButton->setText("Quitter");

    int ret = msgBox.exec();
    return (ret == QMessageBox::Ok);
}



bool MainWindow::copyDirectoryWithRobocopy(const QString& sourceDir, const QString& destDir)
{
    QDir src(sourceDir);

    QDir dirUser(Config::CUSTOM_DATA_PATH);

    while (!dirUser.exists()) {
        if (!checkInternetConnection()) {
            std::exit(0);
        }
    }

    QString program = "robocopy";
    QStringList arguments;
    arguments << sourceDir << destDir << "/MIR" << "/XO";

    QProcess process;
    // Start robocopy
    process.start(program, arguments);

    // wait the process 
    if (!process.waitForFinished(-1)) {
        qDebug() << "Robocopy did not finish within the allotted time.";
        return false;
    }

    QString stdOutput = process.readAllStandardOutput();
    QString stdError = process.readAllStandardError();

    int exitCode = process.exitCode();

    return (exitCode < 8);
}

void MainWindow::onTabChanged(int index)
{
    ui->pushButton_3->setVisible(index ? false : true);
}

QString MainWindow::getSortCriteriaString(SortRole role) const
{
    // attribute roles and names to fill the comboBox
    switch (role) {
    case SortRole::Alphabetical:
        return tr(" Ordre Alphabétique");
    case SortRole::ID:
        return tr(" Ordre ID");
    case SortRole::Favorite:
        return tr(" Ordre Favoris");
    case SortRole::Type:
        return tr(" Ordre Type");
    default:
        return QString();
    }
}

void MainWindow::setupSortCriteria()
{
    /*
     * We needed both enums to have a link between what is written and what is used,
     * we could hack it with some cpp+qt magic (I tried, it's really easy) but I prefer not to.
     */

    //sortCriteria.clear();

    sortCriteria = {
        { getSortCriteriaString(SortRole::Alphabetical), SortRole::Alphabetical },
        { getSortCriteriaString(SortRole::Favorite), SortRole::Favorite },
    };

    // add them in the comboBox directly
    if (ui->comboBox) {
        ui->comboBox->clear();
        for (const auto& criteria : sortCriteria) {
            ui->comboBox->addItem(criteria.name);
        }
    }
}

bool MainWindow::initDirs()
{
    // create the folders if they don't exist already
    QDir userDir(Config::USER_DATA_PATH);
    if (!userDir.exists() && !userDir.mkpath(".")) {
        qWarning() << "Failed to create user data directory";
        return false;
    }

    QDir customDir(Config::CUSTOM_DATA_PATH);
    if (!customDir.exists() && !customDir.mkpath(".")) {
        qWarning() << "Failed to create custom data directory";
        return false;
    }

    // create necessary files with empty arrays if they don't exist
    QFile userFile(getUserFilePath());
    if (!userFile.exists() && userFile.open(QIODevice::WriteOnly)) {
        userFile.write("[]");
        userFile.close();
    }

    QFile customFile(getCustomEntriesPath());
    if (!customFile.exists() && customFile.open(QIODevice::WriteOnly)) {
        customFile.write("[]");
        customFile.close();
    }

    return true;
}

bool MainWindow::loadData()
{
    if(!initDirs()) {
        QMessageBox::critical(this, tr("Error"),
                              tr("N'a pas pu charger les dossiers"));
        return false;
    }

    QSet<int> favoriteIds;

    QString defaultPathFix = QString(Config::DEFAULT_JSON_PATH) + "/" + Config::DEFAULT_FILENAME;

    QFile defaultFile(defaultPathFix);
    if(!defaultFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Erreur"),
                              tr("N'a pas pu ouvrir le fichier par défaut: %1").arg(defaultFile.errorString()));
        return false;
    }

    // read file content once
    QByteArray defaultData = defaultFile.readAll();
    defaultFile.close();

    QJsonParseError parseError;
    QJsonDocument defaultDoc = QJsonDocument::fromJson(defaultData,&parseError);
    if (parseError.error != QJsonParseError::NoError) {
        QMessageBox::critical(this, tr("Erreur"),
                              tr("N'a pas pu lire les données par défaut: %1").arg(parseError.errorString()));
        defaultFile.close();
        return false;
    }

    if (!defaultDoc.isArray()) {
        QMessageBox::critical(this,tr("Erreur"),
                              tr("Format de donnée invalid dans le fichier par défaut"));
        defaultFile.close();
        return false;
    }

    jsonArray = defaultDoc.array();

    QFile userFile(getUserFilePath());
    if (!userFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open user file:" << userFile.errorString();
    } else {
        QByteArray userData = userFile.readAll();
        userFile.close();

        QJsonDocument userDoc = QJsonDocument::fromJson(userData,&parseError);
        if (parseError.error != QJsonParseError::NoError) {
            QMessageBox::warning(this, tr("Warning"),
                                  tr("Failed to parse user favorites: %1").arg(parseError.errorString()));
        } else if (!userDoc.isArray()) {
            QMessageBox::warning(this, tr("Warning"),
                                  tr("Invalid format in user favorites file"));
        } else {
            userJsonArray = userDoc.array();
            favoriteIds.reserve(userJsonArray.size());
            qDebug() << "Loaded" << userJsonArray.size() << "user favorites";
        }
    }

    // Load custom entries
    QFile customFile(getCustomEntriesPath());
    if (!customFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open custom entries file:" << customFile.errorString();
        // Not critical - might be first run
    } else {
        QByteArray customData = customFile.readAll();
        customFile.close();

        QJsonDocument customDoc = QJsonDocument::fromJson(customData, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            QMessageBox::warning(this, tr("Warning"),
                                  tr("Failed to parse custom entries: %1").arg(parseError.errorString()));
        } else if (!customDoc.isArray()) {
            QMessageBox::warning(this, tr("Warning"),
                                  tr("Invalid format in custom entries file"));
        } else {
            customJsonArray = customDoc.array();
            qDebug() << "Loaded" << customJsonArray.size() << "custom entries";
        }
    }

    // synchronize favorite status
    for (const QJsonValue &value : userJsonArray) {
        if (value.isDouble() || value.isString()) {
            favoriteIds.insert(value.toInt());
        }
    }

    // update favorite status in default entries
    for (int i = 0; i < jsonArray.size(); ++i) {
        QJsonObject obj = jsonArray[i].toObject();
        int id = obj["ID"].toInt();
        bool isFavorite = favoriteIds.contains(id);
        obj["Favorite"] = isFavorite;
        jsonArray[i] = obj;
    }

    // update favorite status in custom entries
    for (int i = 0; i < customJsonArray.size(); ++i) {
        QJsonObject obj = customJsonArray[i].toObject();
        int id = obj["ID"].toInt();
        bool isFavorite = favoriteIds.contains(id);
        obj["Favorite"] = isFavorite;
        customJsonArray[i] = obj;
    }

    return true;
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    // set role on comboBox change
    if (index >= 0 && index < sortCriteria.size()) {
        currentSortRole = sortCriteria[index].role;
        populateList();
    }
}

void MainWindow::onSortOrderChanged()
{
    // gui update order change
    currentSortOrder = (currentSortOrder == Qt::AscendingOrder) ?
                           Qt::DescendingOrder : Qt::AscendingOrder;
    ui->pushButton_3->setText(currentSortOrder == Qt::AscendingOrder ? " ↑ " : " ↓ ");
    populateList();
}


void MainWindow::populateList()
{
    // store existing items
    QHash<int, QPair<QListWidgetItem*, QListWidgetItem*>> existingItems;

    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem* item = ui->listWidget->item(i);
        QListWidgetItem* favItem = ui->listWidget_2->item(i);
        if (auto* widget = qobject_cast<richitem*>(ui->listWidget->itemWidget(item))) {
            existingItems[widget->property("id").toInt()] = qMakePair(item, favItem);
        }
    }

    // clear lirts before reordering
    ui->listWidget->clear();
    ui->listWidget_2->clear();
    existingItems.clear();

    // combine arrays and sort
    QJsonArray combinedArray = jsonArray;
    for (const QJsonValue &value : customJsonArray) {
        combinedArray.append(value);
    }

    QList<QJsonObject> sortedList;
    sortedList.reserve(combinedArray.size());
    for (const QJsonValue &value : combinedArray) {
        sortedList.append(value.toObject());
    }


    // sort based on current criteria
    std::sort(sortedList.begin(), sortedList.end(),
              [this](const QJsonObject &a, const QJsonObject &b) {
                  return compareItems(a, b);
              });

    QList<QPair<int, QString>> specialItems;
    // populate lists with sorted items
    for (const QJsonObject &obj : sortedList) {
        int id = obj["ID"].toInt();
        QString path = obj["Path"].toString();
        QString text = obj["Name"].toString();
        QString imagepath = obj["Image"].toString();
        bool favorite = obj["Favorite"].toBool();
        bool custom = obj["Custom"].toBool();
        QString dateStr = obj["Date"].toString();
        if (!custom) {
            imagepath = QString(Config::DEFAULT_IMAGES_PATH) + "/" + imagepath;
        }

        if (!QFile::exists(imagepath)) {

            qDebug() << "L'image n'existe pas:" << imagepath;

            imagepath = ":/icons/icons/no_image.png";
        }

        QIcon icon(imagepath);

        // create main list item
        QListWidgetItem *item = new QListWidgetItem(icon, text);
        richitem *richItemWidget = new richitem(this, id, path, text, imagepath, custom, favorite);

        QListWidgetItem* favItem = new QListWidgetItem(icon, text);
        richitem* favRichItemWidget = new richitem(this, id, path, text, imagepath, custom, favorite);

        richItemWidget->setProperty("id", id);

        if (!custom) {
            QDate today = QDate::currentDate();

            QDate date = QDate::fromString(dateStr, "dd.MM.yyyy");

            if (date > today) {
                continue;
            }

            if (date.addDays(7) >= today)
            {
                richItemWidget->setIconItemAsNew();
                favRichItemWidget->setIconItemAsNew();
                specialItems.append(qMakePair(id, text));
            }
        }

        item->setSizeHint(richItemWidget->size());
        item->setText(text);
        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item, richItemWidget);

        // create favorite list item
        favRichItemWidget->setProperty("id", id);
        favItem->setSizeHint(favRichItemWidget->size());
        favItem->setData(Qt::UserRole + 1, favorite);
        ui->listWidget_2->addItem(favItem);
        ui->listWidget_2->setItemWidget(favItem, favRichItemWidget);
        favItem->setHidden(!favorite);
        // connect signals
        connect(richItemWidget, &richitem::favoriteToggled,
                [this, favItem, favRichItemWidget](bool favorite) {
                    favItem->setHidden(!favorite);
                    favRichItemWidget->setFavorite(favorite);
                    favItem->setData(Qt::UserRole + 1, favorite);
                });

        connect(favRichItemWidget, &richitem::favoriteToggled,
                [this, item, richItemWidget, favItem](bool favorite) {
                    richItemWidget->setFavorite(favorite);
                    favItem->setData(Qt::UserRole + 1, favorite);
                });

        connect(richItemWidget, &richitem::favoriteToggled,
                this, &MainWindow::onFavoriteToggled);
        connect(favRichItemWidget, &richitem::favoriteToggled,
                this, &MainWindow::onFavoriteToggled);
    }

    ui->listWidget->sortItems(currentSortOrder);

    // Now, sort the specialItems (id, text) array alphabetically by text.
    std::sort(specialItems.begin(), specialItems.end(), [](const QPair<int, QString>& a, const QPair<int, QString>& b) {
        return a.second.compare(b.second, Qt::CaseInsensitive) < 0;
        });

    for (int k = 0; k < specialItems.size(); ++k) {
        int specialId = specialItems[k].first;
        for (int i = 0; i < ui->listWidget->count(); ++i) {
            QListWidgetItem* item = ui->listWidget->item(i);
            richitem* widget = qobject_cast<richitem*>(ui->listWidget->itemWidget(item));
            if (widget && widget->property("id").toInt() == specialId) {
                if (i != k) {
                    bool ok = ui->listWidget->model()->moveRows(QModelIndex(), i, 1, QModelIndex(), k);
                    if (!ok)
                        qDebug() << "Failed to move item with id:" << specialId;
                }
                break;
            }
        }
    }

    ui->listWidget->setIconSize(QSize(35, 35));
    ui->listWidget_2->setIconSize(QSize(35, 35));
}

void MainWindow::updateExistingItem(QListWidgetItem* item,
                                    QListWidgetItem* favItem,
                                    const QJsonObject& obj)
{
    QString path = obj["Path"].toString();
    QString text = obj["Name"].toString();
    QString imagepath = obj["Image"].toString();
    bool favorite = obj["Favorite"].toBool();
    bool custom = obj["Custom"].toBool();;

    if (richitem* widget = qobject_cast<richitem*>(ui->listWidget->itemWidget(item))) {
        widget->updateContent(path, text, imagepath, custom, favorite);
        item->setHidden(!favorite);

        if (richitem* favWidget = qobject_cast<richitem*>(
                ui->listWidget_2->itemWidget(favItem))) {
            favWidget->updateContent(path, text, imagepath, custom, favorite);
            favItem->setHidden(!favorite);
        }
    }
}

void MainWindow::createNewItem(const QJsonObject& obj)
{
    QString path = obj["Path"].toString();
    QString text = obj["Name"].toString();
    QString imagepath = obj["Image"].toString();
    bool favorite = obj["Favorite"].toBool();
    int id = obj["ID"].toInt();
    bool custom = obj["Custom"].toBool();

    // create main list item
    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    richitem *richItemWidget = new richitem(this, id, path, text, imagepath, custom, favorite);
    richItemWidget->setProperty("id", id);
    item->setSizeHint(richItemWidget->sizeHint());
    ui->listWidget->addItem(item);
    ui->listWidget->setItemWidget(item, richItemWidget);
    
    // create favorite list item
    QListWidgetItem *favItem = new QListWidgetItem(ui->listWidget_2);
    richitem *favRichItemWidget = new richitem(this, id, path, text, imagepath, custom, favorite);
    favRichItemWidget->setProperty("id", id);
    favItem->setSizeHint(favRichItemWidget->sizeHint());
    ui->listWidget_2->addItem(favItem);
    ui->listWidget_2->setItemWidget(favItem, favRichItemWidget);
    favItem->setHidden(!favorite);

    // connect signals
    connect(richItemWidget, &richitem::favoriteToggled,
            [this, favItem, favRichItemWidget](bool favorite) {
                favItem->setHidden(!favorite);
                favRichItemWidget->setFavorite(favorite);
            });

    connect(favRichItemWidget, &richitem::favoriteToggled,
            [this, item, richItemWidget](bool favorite) {
                richItemWidget->setFavorite(favorite);
                if (!favorite) {
                    item->setHidden(true);
                }
            });

    connect(richItemWidget, &richitem::favoriteToggled,
            this, &MainWindow::onFavoriteToggled);
    connect(favRichItemWidget, &richitem::favoriteToggled,
            this, &MainWindow::onFavoriteToggled);
}

bool MainWindow::compareItems(const QJsonObject &a, const QJsonObject &b) const
{
    // compare each roles to make the order of the lists right
    switch (currentSortRole) {
    case SortRole::Alphabetical:
        return a["Name"].toString().compare(b["Name"].toString(),
                                            Qt::CaseInsensitive) < 0;
    case SortRole::ID:
        return a["ID"].toInt() < b["ID"].toInt();
    case SortRole::Favorite: {
        if (a["Favorite"].toBool() != b["Favorite"].toBool()) {
            return a["Favorite"].toBool() > b["Favorite"].toBool();
        }
        if (a["Favorite"].toBool()) {
            int idxA = -1, idxB = -1;
            for (int i = 0; i < userJsonArray.size(); ++i) {
                if (userJsonArray[i].toInt() == a["ID"].toInt()) idxA = i;
                if (userJsonArray[i].toInt() == b["ID"].toInt()) idxB = i;
            }
            return idxA < idxB;
        }
        return false;
    }
    case SortRole::Type:
        // add type comparison if you want to implement it.
        return false;
    default:
        return false;
    }
}

void MainWindow::on_lineEdit_textChanged(const QString& arg1)
{
     QString searchTerm = arg1.trimmed();

     for (int i = 0; i < ui->listWidget->count(); ++i) {
         QListWidgetItem* item = ui->listWidget->item(i);

         if (searchTerm.isEmpty()) {
             item->setHidden(false);
         }
         else {
             bool match = item->text().contains(searchTerm, Qt::CaseInsensitive);
             item->setHidden(!match);
         }

         if (QListWidgetItem* favItem = ui->listWidget_2->item(i)) {
             bool isFavorite = favItem->data(Qt::UserRole + 1).toBool();
             bool match = favItem->text().contains(searchTerm, Qt::CaseInsensitive);

             if (!isFavorite) {
                 favItem->setHidden(true);
             }
             else {
                 if (searchTerm.isEmpty()) {
                     favItem->setHidden(false);
                 }
                 else {
                     favItem->setHidden(!match);
                 }
             }
         }
     }
}

void MainWindow::clearSearch()
{
    // clear the lineEdit
    ui->lineEdit->clear();
    on_lineEdit_textChanged("");
}

void MainWindow::resetListVisibility()
{
    // reset of the list in case it didn't pick the change
    on_lineEdit_textChanged(ui->lineEdit->text());
}

void MainWindow::saveUserFile()
{
    // create backup before making changes
    QString backupPath;
    if (!createBackup()) {
        QMessageBox::warning(this, tr("Warning"),
                              tr("Failed to create backup before saving. Proceeding with caution."));
    } else {
        backupPath = getLatestBackupPath();
    }

    QFileInfo fileInfo(getUserFilePath());
    if (!ensureDirectoryExists(QFileInfo(getUserFilePath()).absolutePath())) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Failed to create directory for user file"));
        return;
    }

    QSaveFile file(getUserFilePath());
    if (!file.open(QIODevice::WriteOnly)) {
        QString errorMsg = tr("Failed to open user file for writing: %1").arg(file.errorString());
        QMessageBox::critical(this, tr("Error"), errorMsg);
        qWarning() << errorMsg;
        // try to restore from backup if we have one
        if (!backupPath.isEmpty() && QFile::exists(backupPath)) {
            if (restoreFromBackup(backupPath)) {
                QMessageBox::information(this, tr("Recovery"),
                                          tr("Successfully restored from backup."));
            }
        }
        return;
    }

    QJsonDocument doc(userJsonArray);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    qint64 bytesWritten = file.write(jsonData);
    if (bytesWritten == -1 || bytesWritten != jsonData.size()) {
        QString errorMsg = tr("Failed to write user data: %1").arg(file.errorString());
        QMessageBox::critical(this, tr("Error"), errorMsg);
        qWarning() << errorMsg;
        // QSaveFile automatically discards changes on destruction
        if (!backupPath.isEmpty()) {
            restoreFromBackup(backupPath);
        }
        return;
    }

    // commit the changes
    if (!file.commit()) {
        QString errorMsg = tr("N'a pas réussi à sauvegarder les données d'utilisateur: %1").arg(file.errorString());
        QMessageBox::critical(this, tr("Error"), errorMsg);
        qWarning() << errorMsg;
        if(!backupPath.isEmpty()) {
            restoreFromBackup(backupPath);
        }
        return;
    }

    // verify the save
    if (!verifyJsonArray(getUserFilePath())) {
        QString errorMsg = tr("Failed to verify user data after save");
        QMessageBox::critical(this, tr("Error"), errorMsg);
        qWarning() << errorMsg;
        if (!backupPath.isEmpty()) {
            if (restoreFromBackup(backupPath)) {
                QMessageBox::information(this, tr("Recovery"),
                            tr("Successfully restored from backup after verification failure."));
            }
        }
        return;
    }

    // create new backup after successful save
    if (!createBackup()) {
        QMessageBox::warning(this,tr("Warning"),
                             tr("N'as pas pu créer le backup après vérification"));
    }
}

QString MainWindow::getLatestBackupPath() const
{
    QDir backupDir(getBackupPath());
    QStringList filters;
    filters << "*_user.json";
    QFileInfoList files = backupDir.entryInfoList(filters, QDir::Files, QDir::Time);
    return files.isEmpty() ? QString() : files.first().absoluteFilePath();
}

void MainWindow::saveCustomEntriesFile()
{
    // skip if no data to save
    if (customJsonArray.isEmpty()) {
        qDebug() << "No custom entries to save";
        return;
    }

    // ensure directory exists
    QFileInfo fileInfo(getCustomEntriesPath());
    if (!ensureDirectoryExists(fileInfo.absolutePath())) {
        showSaveError(tr("Failed to create custom entries directory"));
        return;
    }

    QFile file(getCustomEntriesPath());
    if (!file.open(QIODevice::WriteOnly)) {
        showSaveError(tr("Failed to open custom entries file for writing"));
        qWarning() << "Failed to open custom entries file:" << file.errorString();
        return;
    }

    // create the document and write
    QJsonDocument doc(customJsonArray);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);

    qDebug() << "Writing entries:";
    for (const QJsonValue &value : customJsonArray) {
        QJsonObject obj = value.toObject();
        qDebug() << "  ID:" << obj["ID"].toInt()
                 << "Name:" << obj["Name"].toString()
                 << "Favorite:" << obj["Favorite"].toBool();
    }

    if (file.write(jsonData) == -1) {
        showSaveError(tr("Failed to write custom entries data"));
        qWarning() << "Failed to write custom entries:" << file.errorString();
        file.close();
        return;
    }

    file.close();

    // verify the save
    if (!verifyJsonArray(getCustomEntriesPath())) {
        showSaveError(tr("Failed to verify custom entries after save"));
        return;
    }

    qDebug() << "Successfully saved" << customJsonArray.size() << "custom entries";

    /*
     *  try to create backup but don't fail if backup fails
     *  implement more if you need we never discussed how to
     *  handle such cases
     */
    if (!createBackup()) {
        qWarning() << "Failed to create backup after saving custom entries";
    }
}

QString MainWindow::copyImageToStorage(const QString &originalImagePath)
{
    /*
     * copy the image to the correct folder
     */

    if (originalImagePath.isEmpty() || !QFile::exists(originalImagePath)) {
        QMessageBox::warning(this, tr("Error"),
              tr("Invalid image path: %1").arg(originalImagePath));
        return QString();
    }

    QFileInfo fileInfo(originalImagePath);
    if (!fileInfo.isReadable()) {
        QMessageBox::warning(this,tr("Erreur"),
                              tr("N'a pas pu lire l'image: %1").arg(originalImagePath));
        return QString();
    }

    QString imagesDir = getCustomImagesPath();
    if (!ensureDirectoryExists(imagesDir)) {
        QMessageBox::warning(this, tr("Erreur"),
                             tr("N'a pas pu créer le dossier d'images"));
        return QString();
    }

    QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch());
    QString newFileName = timestamp + "_" + fileInfo.fileName();
    QString destinationPath = QDir(imagesDir).filePath(newFileName);

    QFile destFile(destinationPath);
    if (destFile.exists() && !destFile.remove()) {
        QMessageBox::warning(this, tr("Error"),
                                  tr("Failed to replace existing image: %1").arg(destFile.errorString()));
        return QString();
    }

    if (!QFile::copy(originalImagePath, destinationPath) || !QFile::exists(destinationPath)) {
        QMessageBox::warning(this, tr("Error"),
                              tr("Failed to copy image file: %1").arg(destinationPath));
        return QString();
    }

    return destinationPath;
}

bool MainWindow::verifyJsonArray(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    return doc.isArray();
}

bool MainWindow::createBackup() const
{
    QString backupDir = getBackupPath();

    // ensure backup directory exists
    if (!ensureDirectoryExists(backupDir)) {
        qWarning() << "Failed to create/access backup directory:" << backupDir;
        return false;
    }

    QString timestamp = generateBackupFileName();

    bool backupSuccess = true;

    // backup user file
    if (QFile::exists(getUserFilePath())) {
        QString userBackup = QDir(backupDir).filePath(timestamp + "_user.json");
        qDebug() << "Attempting to backup user file:";
        qDebug() << "  Source:" << getUserFilePath();
        qDebug() << "  Destination:" << userBackup;

        // first check if source is readable
        QFile sourceFile(getUserFilePath());
        if (sourceFile.open(QIODevice::ReadOnly)) {
            sourceFile.close();

            // remove destination file if it exists
            if (QFile::exists(userBackup)) {
                QFile::remove(userBackup);
            }

            if (!QFile::copy(getUserFilePath(), userBackup)) {
                qWarning() << "Failed to backup user file:";
                qWarning() << "  Source exists:" << QFile::exists(getUserFilePath());
                qWarning() << "  Source readable:" << QFileInfo(getUserFilePath()).isReadable();
                qWarning() << "  Destination dir writable:" << QFileInfo(backupDir).isWritable();
                backupSuccess = false;
            } else {
                qDebug() << "Successfully backed up user file";
            }
        } else {
            qWarning() << "Source file not readable:" << sourceFile.errorString();
            backupSuccess = false;
        }
    } else {
        qDebug() << "User file does not exist, skipping backup";
    }

    // backup custom entries file
    if (QFile::exists(getCustomEntriesPath())) {
        QString customBackup = QDir(backupDir).filePath(timestamp + "_custom.json");
        qDebug() << "Attempting to backup custom entries file:";
        qDebug() << "  Source:" << getCustomEntriesPath();
        qDebug() << "  Destination:" << customBackup;

        // first check if source is readable
        QFile sourceFile(getCustomEntriesPath());
        if (sourceFile.open(QIODevice::ReadOnly)) {
            sourceFile.close();

            // remove destination file if it exists
            if (QFile::exists(customBackup)) {
                QFile::remove(customBackup);
            }

            if (!QFile::copy(getCustomEntriesPath(), customBackup)) {
                qWarning() << "Failed to backup custom entries file:";
                qWarning() << "  Source exists:" << QFile::exists(getCustomEntriesPath());
                qWarning() << "  Source readable:" << QFileInfo(getCustomEntriesPath()).isReadable();
                qWarning() << "  Destination dir writable:" << QFileInfo(backupDir).isWritable();
                backupSuccess = false;
            } else {
                qDebug() << "Successfully backed up custom entries file";
            }
        } else {
            qWarning() << "Source file not readable:" << sourceFile.errorString();
            backupSuccess = false;
        }
    } else {
        qDebug() << "Custom entries file does not exist, skipping backup";
    }

    // cleanup old backups only if we had a successful backup
    if (backupSuccess) {
        QDir dir(backupDir);
        QStringList filters;
        filters << "*_user.json" << "*_custom.json";
        QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Time | QDir::Reversed);

        while (files.size() > 6) { // Keep 3 pairs of files (6 total files)
            QString fileToRemove = files.first().absoluteFilePath();
            if (QFile::remove(fileToRemove)) {
                qDebug() << "Removed old backup:" << fileToRemove;
            } else {
                qWarning() << "Failed to remove old backup:" << fileToRemove;
            }
            files.removeFirst();
        }
    }

    return backupSuccess;
}

bool MainWindow::restoreFromBackup(const QString &backupFile)
{
    QFileInfo backupInfo(backupFile);
    if (!backupInfo.exists()) {
        showSaveError(tr("Backup file does not exist"));
        return false;
    }

    // parse timestamp from filename
    QString timestamp = backupInfo.baseName().section('_', 0, -2);
    QString backupDir = backupInfo.absolutePath();

    // restore user file
    QString userBackup = QDir(backupDir).filePath(timestamp + "_user.json");
    if (QFile::exists(userBackup)) {
        if (!QFile::remove(getUserFilePath())) {
            showSaveError(tr("Failed to remove existing user file"));
            return false;
        }
        if (!QFile::copy(userBackup, getUserFilePath())) {
            showSaveError(tr("Failed to restore user file from backup"));
            return false;
        }
    }

    // restore custom entries
    QString customBackup = QDir(backupDir).filePath(timestamp + "_custom.json");
    if (QFile::exists(customBackup)) {
        if (!QFile::remove(getCustomEntriesPath())) {
            showSaveError(tr("Failed to remove existing custom entries file"));
            return false;
        }
        if (!QFile::copy(customBackup, getCustomEntriesPath())) {
            showSaveError(tr("Failed to restore custom entries from backup"));
            return false;
        }
    }

    return true;
}

QString MainWindow::generateBackupFileName() const
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
}

bool MainWindow::ensureDirectoryExists(const QString &path) const
{
    QDir dir(path);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            showSaveError(tr("Failed to create directory: %1").arg(path));
            return false;
        }
    }
    return true;
}

void MainWindow::addCustomEntry(const QJsonObject &entry)
{
    QJsonObject modifiedEntry = entry;
    QString originalImagePath = entry["Image"].toString();
    QString newImagePath = copyImageToStorage(originalImagePath);

    if (!newImagePath.isEmpty()) {
        modifiedEntry["Image"] = newImagePath;
    }

    modifiedEntry["Favorite"] = true;

    if (modifiedEntry["ID"].toInt() == -1) {
        modifiedEntry["ID"] = DialogAddItem().generateUniqueId(customJsonArray);
    }
    int newEntryId = modifiedEntry["ID"].toInt();

    customJsonArray.append(modifiedEntry);
    userJsonArray.append(newEntryId);

    updateJsonArrays();
    populateList();
    on_lineEdit_textChanged(ui->lineEdit->text());
}

void MainWindow::updateJsonArrays()
{
    QSet<int> favoriteIds;

    // extract favorite IDs from userJsonArray
    for (const QJsonValue &value : userJsonArray) {
        if (value.isDouble() || value.isString()) {
            favoriteIds.insert(value.toInt());
        }
    }

    qDebug() << "Collected favoriteIds:" << favoriteIds;

    // update jsonArray with favorite status
    for (int i = 0; i < jsonArray.size(); ++i) {
        QJsonObject obj = jsonArray[i].toObject();
        int id = obj["ID"].toInt();
        bool wasFavorite = obj["Favorite"].toBool();
        obj["Favorite"] = favoriteIds.contains(id);
        jsonArray[i] = obj;
        if (wasFavorite != obj["Favorite"].toBool()) {
            qDebug() << "Changed favorite status for default entry" << id
                     << "from" << wasFavorite << "to" << obj["Favorite"].toBool();
        }
    }

    // process custom entries - always set as favorites if in the favorites list
    for (int i = 0; i < customJsonArray.size(); ++i) {
        QJsonObject obj = customJsonArray[i].toObject();
        int id = obj["ID"].toInt();
        bool wasFavorite = obj["Favorite"].toBool();

        // custom entries favorite status is based on presence in favoriteIds
        obj["Favorite"] = favoriteIds.contains(id);

        customJsonArray[i] = obj;
    }

}

void MainWindow::onFavoriteToggled(bool favorite)
{
    richitem *senderWidget = qobject_cast<richitem*>(sender());
    if (!senderWidget) {
        qWarning() << "onFavoriteToggled called with invalid sender";
        return;
    }

    int id = senderWidget->property("id").toInt();

    // check if this is a custom entry being unfavorited from favorites tab
    bool isCustomEntry = false;
    for (const QJsonValue &value : customJsonArray) {
        if (value.toObject()["ID"].toInt() == id) {
            isCustomEntry = true;
            break;
        }
    }

    // get the tab index to check if we're in favorites tab
    bool inFavoritesTab = (ui->tabWidget->currentIndex() == 1);


    updateUserJsonArray(id, favorite);

    // Update the availableList item that matches the given ID
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem* item = ui->listWidget->item(i);
        richitem* widget = qobject_cast<richitem*>(ui->listWidget->itemWidget(item));
        if (widget && widget->property("id").toInt() == id) {
            widget->setFavorite(favorite);
            break;
        }
    }

    // Update the favoritesList item that matches the given ID
    for (int i = 0; i < ui->listWidget_2->count(); ++i) {
        QListWidgetItem* favItem = ui->listWidget_2->item(i);
        richitem* favWidget = qobject_cast<richitem*>(ui->listWidget_2->itemWidget(favItem));
        if (favWidget && favWidget->property("id").toInt() == id) {
            favWidget->setFavorite(favorite);
            // Hide or show the item depending on its favorite state
            favItem->setHidden(!favorite);
            break;
        }
    }

    updateJsonArrays();

    // Update current filter if search is active
    QString currentFilter = ui->lineEdit->text();
    if (!currentFilter.isEmpty()) {
        on_lineEdit_textChanged(currentFilter);
    }
}

void MainWindow::updateUserJsonArray(int id, bool favorite)
{
    if (favorite) {
        // check if ID already exists
        bool idExists = false;
        for (const QJsonValue &value : userJsonArray) {
            if (value.toInt() == id) {
                idExists = true;
                break;
            }
        }

        if (!idExists) {
            userJsonArray.append(id);
            qDebug() << "Added ID" << id << "to userJsonArray";
        }
    } else {
        // remove from favorites
        for (int i = 0; i < userJsonArray.size(); ++i) {
            if (userJsonArray[i].toInt() == id) {
                userJsonArray.removeAt(i);
                qDebug() << "Removed ID" << id << "from userJsonArray";
                break;
            }
        }
    }
}

void MainWindow::updateCustomEntry(const QJsonObject& updatedEntry)
{
    int id = updatedEntry["ID"].toInt();
    QJsonObject oldEntry;
    bool found = false;

    // Find existing entry in customJsonArray
    for (int i = 0; i < customJsonArray.size(); ++i) {
        QJsonObject obj = customJsonArray[i].toObject();
        if (obj["ID"].toInt() == id) {
            oldEntry = obj;
            found = true;
            break;
        }
    }

    QJsonObject modifiedEntry = updatedEntry;

    // If the image has been modified (or the entry did not yet exist)
    // then we copy the new image into the storage folder
    if (!found || updatedEntry["Image"].toString() != oldEntry["Image"].toString()) {
        QString originalImagePath = updatedEntry["Image"].toString();
        QString newImagePath = copyImageToStorage(originalImagePath);
        if (!newImagePath.isEmpty()) {
            modifiedEntry["Image"] = newImagePath;

            // Delete the old image only if it is in the custom_images folder
            QString oldImagePath = oldEntry["Image"].toString();
            if (!oldImagePath.isEmpty() && QFile::exists(oldImagePath)) {
                // Check that the path starts with the image storage folder
                QString customImagesPath = getCustomImagesPath();
                if (oldImagePath.startsWith(customImagesPath)) {
                    if (QFile::remove(oldImagePath)) {
                        qDebug() << "Old image deleted:" << oldImagePath;
                    }
                    else {
                        qWarning() << "Failed to delete old image:" << oldImagePath;
                    }
                }
            }
        }
    }

    modifiedEntry["Favorite"] = true;

    // Update entry in customJsonArray or add if not exist
    if (found) {
        for (int i = 0; i < customJsonArray.size(); ++i) {
            QJsonObject obj = customJsonArray[i].toObject();
            if (obj["ID"].toInt() == id) {
                customJsonArray[i] = modifiedEntry;
                break;
            }
        }
    }
    else {
        customJsonArray.append(modifiedEntry);
        userJsonArray.append(id);
    }

    updateJsonArrays();
    saveUserFile();
    saveCustomEntriesFile();
    populateList();
    on_lineEdit_textChanged(ui->lineEdit->text());
}

void MainWindow::onCustomEntryDeleteRequested(int id)
{
    // Remove entry from customJsonArray
    for (int i = customJsonArray.size() - 1; i >= 0; --i) {
        QJsonObject obj = customJsonArray[i].toObject();
        if (obj["ID"].toInt() == id) {
            // Delete the associated image if it's in the custom_images folder
            QString imagePath = obj["Image"].toString();
            QString customImagesPath = getCustomImagesPath();
            if (!imagePath.isEmpty() && QFile::exists(imagePath) && imagePath.startsWith(customImagesPath)) {
                if (QFile::remove(imagePath)) {
                    qDebug() << "Old image deleted:" << imagePath;
                }
                else {
                    qWarning() << "Failed to delete old image:" << imagePath;
                }
            }
            customJsonArray.removeAt(i);
        }
    }

    // Remove the id from userJsonArray
    for (int i = userJsonArray.size() - 1; i >= 0; --i) {
        if (userJsonArray[i].toInt() == id) {
            userJsonArray.removeAt(i);
        }
    }

    updateJsonArrays();
    populateList();
    on_lineEdit_textChanged(ui->lineEdit->text());

    qDebug() << "Entry" << id << "deleted.";
}

void MainWindow::processMarkedForDeletionEntries() {
    if (customEntriesMarkedForDeletion.isEmpty()) {
        return;
    }

    // remove from customJsonArray
    for (int i = customJsonArray.size() - 1; i >= 0; --i) {
        if (customEntriesMarkedForDeletion.contains(
                customJsonArray[i].toObject()["ID"].toInt())) {
            customJsonArray.removeAt(i);
        }
    }

    // remove from userJsonArray
    for (int i = userJsonArray.size() - 1; i >= 0; --i) {
        if (customEntriesMarkedForDeletion.contains(userJsonArray[i].toInt())) {
            userJsonArray.removeAt(i);
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionParam_tres_triggered()
{
    // dialog for the settings
    Settings *settings = new Settings(this);
    settings->setAttribute(Qt::WA_DeleteOnClose);
    settings->setFixedSize(settings->size());
    settings->exec();
}

void MainWindow::on_actionManageFav_triggered()
{
    DialogAddFavorites dlg(this, this);
    if (dlg.exec() == QDialog::Accepted) {
        QJsonArray newFavorites = dlg.getSelectedFavorites();

        // Update with new favs
        QSet<int> newFavSet;
        for (const QJsonValue& value : newFavorites) {
            int appId = value.toInt();
            newFavSet.insert(appId);
            updateUserJsonArray(appId, true);
        }

        for (int i = userJsonArray.size() - 1; i >= 0; --i) {
            int id = userJsonArray[i].toInt();
            if (!newFavSet.contains(id)) {
                updateUserJsonArray(id, false);
            }
        }
        userJsonArray = newFavorites;

        updateJsonArrays();
        populateList();
    }
}

void MainWindow::on_actionAide_triggered()
{
    bool started = QProcess::startDetached("cmd.exe", { "/c", "start", "", "chrome", Config::USER_GUIDE_PATH});
}

void MainWindow::on_actionAjouter_triggered()
{
    // dialog for custom items
    DialogAddItem* dialog = new DialogAddItem(this);
    connect(dialog, &DialogAddItem::customEntryCreated, this, [this](const QJsonObject& newEntry) {
        qDebug() << "Received new entry from dialog:" << newEntry;
        addCustomEntry(newEntry);
        }, Qt::QueuedConnection);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setFixedSize(dialog->size());
    dialog->exec();
}

void MainWindow::on_pushButton_clicked()
{
    // dialog for custom items
    DialogAddItem *dialog = new DialogAddItem(this);
    connect(dialog, &DialogAddItem::customEntryCreated, this, [this](const QJsonObject &newEntry) {
        qDebug() << "Received new entry from dialog:" << newEntry;
        addCustomEntry(newEntry);
    }, Qt::QueuedConnection);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setFixedSize(dialog->size());
    dialog->exec();
}

void MainWindow::on_actionApropos_triggered()
{
    Propos* propos = new Propos(this);
    propos->setAttribute(Qt::WA_DeleteOnClose);
    propos->setFixedSize(propos->size());
    propos->exec();
}

void MainWindow::updateFavoriteOrder(const QModelIndex &sourceIndex, int row)
{
    Q_UNUSED(sourceIndex);
    Q_UNUSED(row);

    userJsonArray = QJsonArray();
    for (int i = 0; i < ui->listWidget_2->count(); ++i)
    {
        QListWidgetItem *item = ui->listWidget_2->item(i);
        richitem *widget = qobject_cast<richitem*>(ui->listWidget_2->itemWidget(item));
        if (widget && !item->isHidden())
        {
            userJsonArray.append(widget->property("id").toInt());
        }
    }
}

void MainWindow::on_actionQuitter_triggered()
{
    close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("AppDirectory", "MainWindow");
    settings.setValue("MainWindowGeometry", saveGeometry());
    settings.setValue("MainWindowState", saveState());

    processMarkedForDeletionEntries();
    saveUserFile();
    saveCustomEntriesFile();
    createBackup();
    qApp->quit();
    event->accept();
}
