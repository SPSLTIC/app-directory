#include "DialogAddFavorites.h"
#include "ui_DialogAddFavorites.h"
#include <QEvent>
#include <QMouseEvent>
#include <QListWidgetItem>
#include <QDebug>
#include <QStringList>
#include <QTimer>
#include <QJsonObject>
#include <QDate>

DialogAddFavorites::DialogAddFavorites(MainWindow* mainWindow, QWidget* parent) :
    QDialog(parent),
    m_mainWindow(mainWindow),
    ui(new Ui::DialogAddFavorites)
{
    ui->setupUi(this);

    populateAvailableList(m_mainWindow->jsonArray);
    populateAvailableList(m_mainWindow->customJsonArray);
    populateFavoriteList(m_mainWindow->jsonArray);
    populateFavoriteList(m_mainWindow->customJsonArray);
    sortFavoritesByUserJsonArray(m_mainWindow->userJsonArray);


    ui->availableList->setDragEnabled(true);
    ui->availableList->setAcceptDrops(true);
    ui->availableList->setDropIndicatorShown(true);
    ui->availableList->setDragDropMode(QAbstractItemView::DragDrop);
    ui->availableList->setDefaultDropAction(Qt::CopyAction);
    ui->availableList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    ui->favoritesList->setDragEnabled(true);
    ui->favoritesList->setAcceptDrops(true);
    ui->favoritesList->setDropIndicatorShown(true);
    ui->favoritesList->setDragDropMode(QAbstractItemView::DragDrop);
    ui->favoritesList->setDefaultDropAction(Qt::MoveAction);
    ui->favoritesList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    ui->availableList->setCursor(Qt::PointingHandCursor);
    ui->favoritesList->setCursor(Qt::PointingHandCursor);

    ui->addButton->setEnabled(false);
    ui->removeButton->setEnabled(false);

    QPushButton* cancelButton = ui->buttonBox->button(QDialogButtonBox::Cancel);
    if (cancelButton) {
        cancelButton->setText(tr("Annuler"));
    }

    QPushButton* okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    if (okButton) {
        okButton->setText(tr("Confirmer"));
    }

    connect(ui->favoritesList->model(), &QAbstractItemModel::rowsInserted,
        this, &DialogAddFavorites::onFavoritesRowsInserted);

    connect(ui->availableList->model(), &QAbstractItemModel::rowsInserted,
        this, &DialogAddFavorites::onAvailableRowsInserted);

    // Connect the itemPressed signals for immediate response
    connect(ui->availableList, &QListWidget::itemPressed,
        this, &DialogAddFavorites::onAvailableListItemPressed);
    connect(ui->favoritesList, &QListWidget::itemPressed,
        this, &DialogAddFavorites::onFavoritesListItemPressed);
}

void DialogAddFavorites::populateAvailableList(const QJsonArray& jsonArray)
{
   
    for (const QJsonValue& value : jsonArray) {
        if (!value.isObject())
            continue;

        QJsonObject obj = value.toObject();
        QString appName = obj.value("Name").toString();
        int appId = obj.value("ID").toInt();
        QString imagePath = obj.value("Image").toString(); 
        bool custom = obj.value("Custom").toBool();
        QString dateStr = obj.value("Date").toString();

        if (!QFile::exists(imagePath)) {
            qDebug() << "L'image n'existe pas:" << imagePath;

            imagePath = ":/icons/icons/no_image.png";
        }

        QIcon icon(imagePath);

        QListWidgetItem* item = new QListWidgetItem(icon, appName);

        if (!custom) {
            QDate today = QDate::currentDate();

            QDate date = QDate::fromString(dateStr, "dd.MM.yyyy");

            qDebug() << "DateStr:" << dateStr << "Date:" << date << "Aujourd'hui:" << today;

            if (date > today) {
                continue;
            }
        }

        // Store the unique identifier in the Qt::UserRole role
        item->setData(Qt::UserRole, appId);
   
        if (isFavorite(m_mainWindow->userJsonArray, appId)) {

            ui->availableList->addItem(item);
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        }
        else {
            ui->availableList->addItem(item);
        }
    }
    ui->availableList->setIconSize(QSize(32, 32));
    ui->availableList->sortItems();
}

bool DialogAddFavorites::isFavorite(const QJsonArray& userJsonArray, int idItem) {

    for (const QJsonValue& value : userJsonArray) {

        
        int appId = value.toInt();
        if (appId == idItem) {
            return true;
        }
    }
    return false;
}

void DialogAddFavorites::populateFavoriteList(const QJsonArray& jsonArray)
{
    for (const QJsonValue& value : jsonArray) {
        QJsonObject obj = value.toObject();
        QString appName = obj.value("Name").toString();
        int appId = obj.value("ID").toInt();
        QString imagePath = obj.value("Image").toString();

        if (!value.isObject() || !isFavorite(m_mainWindow->userJsonArray, appId))
            continue;

            if (!QFile::exists(imagePath)) {
                qDebug() << "L'image n'existe pas:" << imagePath;

                imagePath = ":/icons/icons/no_image.png";
            }

            QIcon icon(imagePath);

            QListWidgetItem* itemFav = new QListWidgetItem(icon, appName);
            itemFav->setData(Qt::UserRole, appId);
            ui->favoritesList->addItem(itemFav);
    }

    ui->favoritesList->setIconSize(QSize(32, 32));
}

void DialogAddFavorites::sortFavoritesByUserJsonArray(const QJsonArray& userJsonArray)
{
    // Build a mapping from each ID in userJsonArray to its index (order)
    QHash<int, int> orderMap;
    for (int i = 0; i < userJsonArray.size(); ++i) {
        int id = userJsonArray[i].toInt();
        orderMap.insert(id, i);
    }

    // Extract all items from favoritesList into a list
    QList<QListWidgetItem*> items;
    while (ui->favoritesList->count() > 0) {
        items.append(ui->favoritesList->takeItem(0));
    }

    // Sort items by comparing the order of their IDs from orderMap.
    // Si un item n'existe pas dans userJsonArray, on lui assigne une grande valeur (INT_MAX) pour le placer à la fin.
    std::sort(items.begin(), items.end(), [&orderMap](QListWidgetItem* a, QListWidgetItem* b) {
        int idA = a->data(Qt::UserRole).toInt();
        int idB = b->data(Qt::UserRole).toInt();
        return orderMap.value(idA, INT_MAX) < orderMap.value(idB, INT_MAX);
        });

    // Réinsérer les items triés dans favoritesList
    for (QListWidgetItem* item : items) {
        ui->favoritesList->addItem(item);
    }
}


DialogAddFavorites::~DialogAddFavorites()
{
    delete ui;
}

void DialogAddFavorites::on_addButton_clicked()
{
    QList<QListWidgetItem*> selectedItems = ui->availableList->selectedItems();
    for (QListWidgetItem* item : selectedItems) {
        int id = item->data(Qt::UserRole).toInt();
        bool alreadyAdded = false;

        for (int i = 0; i < ui->favoritesList->count(); ++i) {
            QListWidgetItem* favItem = ui->favoritesList->item(i);
            if (favItem->data(Qt::UserRole).toInt() == id) {
                alreadyAdded = true;
                break;
            }
        }

        if (!alreadyAdded) {
            QListWidgetItem* newItem = new QListWidgetItem(item->icon(), item->text());
            newItem->setData(Qt::UserRole, id);
            ui->favoritesList->addItem(newItem);
            ui->addButton->setEnabled(false);
            // Désactiver l'item dans availableList
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        }
    }
    ui->favoritesList->setIconSize(QSize(32, 32));
}

void DialogAddFavorites::on_removeButton_clicked()
{
    QList<QListWidgetItem*> selectedItems = ui->favoritesList->selectedItems();
    for (QListWidgetItem* item : selectedItems) {

        int id = item->data(Qt::UserRole).toInt();

        int row = ui->favoritesList->row(item);
        QListWidgetItem* removedItem = ui->favoritesList->takeItem(row);
        delete removedItem;
        ui->removeButton->setEnabled(false);


        for (int i = 0; i < ui->availableList->count(); ++i) {
            QListWidgetItem* availItem = ui->availableList->item(i);
            if (availItem->data(Qt::UserRole).toInt() == id) {
                availItem->setFlags(availItem->flags() | Qt::ItemIsEnabled);
            }
        }
    }
}

QJsonArray DialogAddFavorites::getSelectedFavorites() const
{
    QJsonArray favorites;
    for (int i = 0; i < ui->favoritesList->count(); ++i) {
        QListWidgetItem* item = ui->favoritesList->item(i);
        int id = item->data(Qt::UserRole).toInt();
        favorites.append(id);
    }
    return favorites;
}

void DialogAddFavorites::onFavoritesRowsInserted(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent);
    QTimer::singleShot(0, this, [=]() {
        // Part 1: For each new item inserted into favoritesList, disable the corresponding item in availableList
        for (int row = first; row <= last; row++) {
            QListWidgetItem* favItem = ui->favoritesList->item(row);
            if (favItem) {
                int id = favItem->data(Qt::UserRole).toInt();
                // Search for the corresponding item in availableList by comparing the ID
                for (int i = 0; i < ui->availableList->count(); i++) {
                    QListWidgetItem* availItem = ui->availableList->item(i);
                    if (availItem->data(Qt::UserRole).toInt() == id) {
                        if (availItem->flags() & Qt::ItemIsEnabled) {
                            availItem->setFlags(availItem->flags() & ~Qt::ItemIsEnabled);
                            qDebug() << "Disabling the item with ID" << id << "in availableList (drop)";
                        }
                    }
                }
            }
        }

        // Part 2: Remove duplicates from favoritesList (based on the ID)
        QSet<int> seen;
        for (int i = ui->favoritesList->count() - 1; i >= 0; i--) {
            QListWidgetItem* item = ui->favoritesList->item(i);
            int id = item->data(Qt::UserRole).toInt();
            if (seen.contains(id)) {
                qDebug() << "Duplicate detected in favoritesList, removing the item with ID:" << id;
                QListWidgetItem* removed = ui->favoritesList->takeItem(i);
                delete removed;
            }
            else {
                seen.insert(id);
            }
        }
        });
    ui->favoritesList->setIconSize(QSize(32, 32));
    ui->addButton->setEnabled(false);
}

void DialogAddFavorites::onAvailableRowsInserted(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent);
    QTimer::singleShot(0, this, [=]() {
        // Group the items in availableList by ID
        QMap<int, QList<QListWidgetItem*>> groups;
        int total = ui->availableList->count();
        for (int i = 0; i < total; i++) {
            QListWidgetItem* item = ui->availableList->item(i);
            if (item)
                groups[item->data(Qt::UserRole).toInt()].append(item);
        }

        // For each group with multiple items (duplicates)
        for (auto it = groups.begin(); it != groups.end(); ++it) {
            int id = it.key();
            const QList<QListWidgetItem*>& items = it.value();
            if (items.size() > 1) { // s'il y a doublons
                QListWidgetItem* original = nullptr;
                // Chercher l'item original désactivé (celui déjà ajouté)
                for (QListWidgetItem* item : items) {
                    if (!(item->flags() & Qt::ItemIsEnabled)) {
                        original = item;
                        break;
                    }
                }
               
                if (!original) {
                    original = items.first();
                }
               
                original->setFlags(original->flags() | Qt::ItemIsEnabled);

                // Remove all other duplicate items in availableList
                for (QListWidgetItem* item : items) {
                    if (item != original) {
                        int idx = ui->availableList->row(item);
                        QListWidgetItem* dup = ui->availableList->takeItem(idx);
                        delete dup;
                        qDebug() << "Duplication removed in availableList for ID:" << id;
                        ui->removeButton->setEnabled(false);
                    }
                }
                
                for (int i = ui->favoritesList->count() - 1; i >= 0; i--) {
                    QListWidgetItem* favItem = ui->favoritesList->item(i);
                    if (favItem->data(Qt::UserRole).toInt() == id) {
                        int favIdx = ui->favoritesList->row(favItem);
                        QListWidgetItem* removedFav = ui->favoritesList->takeItem(favIdx);
                        delete removedFav;
                        ui->removeButton->setEnabled(false);
                        qDebug() << "Item removed from favoritesList for ID:" << id;
                    }
                }
            }
        }
        });
}

void DialogAddFavorites::on_availableSearch_textChanged(const QString& arg1)
{
    QString searchTerm = arg1.trimmed();
    for (int i = 0; i < ui->availableList->count(); ++i) {
        QListWidgetItem* item = ui->availableList->item(i);
        if (searchTerm.isEmpty()) {
            item->setHidden(false);
        }
        else {
            bool match = item->text().contains(searchTerm, Qt::CaseInsensitive);
            item->setHidden(!match);
        }
    }
}

void DialogAddFavorites::on_favoritesSearch_textChanged(const QString& arg1)
{
    QString searchTerm = arg1.trimmed();
    for (int i = 0; i < ui->favoritesList->count(); ++i) {
        QListWidgetItem* item = ui->favoritesList->item(i);
        if (searchTerm.isEmpty()) {
            item->setHidden(false);
        }
        else {
            bool match = item->text().contains(searchTerm, Qt::CaseInsensitive);
            item->setHidden(!match);
        }
    }
}

void DialogAddFavorites::onAvailableListItemPressed(QListWidgetItem* item)
{
    Q_UNUSED(item);
    // When an item in availableList is pressed, enable the "Add" button
    // and disable the "Remove" button.
    ui->addButton->setEnabled(true);
    ui->removeButton->setEnabled(false);

    qDebug() << "Change on available V2";
}

void DialogAddFavorites::onFavoritesListItemPressed(QListWidgetItem* item)
{
    Q_UNUSED(item);
    // When an item in favoritesList is pressed, enable the "Remove" button
    // and disable the "Add" button.
    ui->removeButton->setEnabled(true);
    ui->addButton->setEnabled(false);

    qDebug() << "Change on favorites V2";
}