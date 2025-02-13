#include "DialogAddFavorites.h"
#include "ui_DialogAddFavorites.h"
#include <QEvent>
#include <QMouseEvent>
#include <QListWidgetItem>
#include <QDebug>
#include <QStringList>
#include <QTimer>
#include <QJsonObject>

DialogAddFavorites::DialogAddFavorites(MainWindow* mainWindow, QWidget* parent) :
    QDialog(parent),
    m_mainWindow(mainWindow),
    ui(new Ui::DialogAddFavorites)
{
    ui->setupUi(this);

    // Remplir la liste des applications disponibles (exemple statique)

    qDebug() << "userJsonArray" << m_mainWindow->userJsonArray;
    //populateFavoriteList(m_mainWindow->userJsonArray);

    populateAvailableList(m_mainWindow->jsonArray);
    populateAvailableList(m_mainWindow->customJsonArray);

    ui->favoritesList->sortItems();

    // Configuration de la liste "availableList" (source) :
    // On veut qu'elle serve uniquement de source de drag, sans accepter de drop.
   // Pour availableList (source) : on souhaite qu'elle permette les drops venant de favoritesList
    ui->availableList->setDragEnabled(true);
    ui->availableList->setAcceptDrops(true);  // On active les drops pour pouvoir réagir au drop depuis favoritesList
    ui->availableList->setDropIndicatorShown(true);
    ui->availableList->setDragDropMode(QAbstractItemView::DragDrop);
    ui->availableList->setDefaultDropAction(Qt::CopyAction);
    ui->availableList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Pour favoritesList (destination)
    ui->favoritesList->setDragEnabled(true);
    ui->favoritesList->setAcceptDrops(true);
    ui->favoritesList->setDropIndicatorShown(true);
    ui->favoritesList->setDragDropMode(QAbstractItemView::DragDrop);
    ui->favoritesList->setDefaultDropAction(Qt::CopyAction);
    ui->favoritesList->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Définir le curseur par défaut
    ui->availableList->setCursor(Qt::PointingHandCursor);
    ui->favoritesList->setCursor(Qt::PointingHandCursor);



    // Définir le curseur par défaut sur les listes
    ui->availableList->setCursor(Qt::PointingHandCursor);
    ui->favoritesList->setCursor(Qt::PointingHandCursor);

    connect(ui->favoritesList->model(), &QAbstractItemModel::rowsInserted,
        this, &DialogAddFavorites::onFavoritesRowsInserted);

    connect(ui->availableList->model(), &QAbstractItemModel::rowsInserted,
        this, &DialogAddFavorites::onAvailableRowsInserted);

}

void DialogAddFavorites::populateAvailableList(const QJsonArray& jsonArray)
{
    //ui->availableList->clear();  // On vide la liste avant de la remplir

    for (const QJsonValue& value : jsonArray) {
        if (!value.isObject())
            continue;  // On ignore si ce n'est pas un objet

        QJsonObject obj = value.toObject();
        QString appName = obj.value("Name").toString();
        int appId = obj.value("ID").toInt();

        qDebug() << "isFavorite" << isFavorite(m_mainWindow->userJsonArray, appId);
        // Créer l'item avec le nom de l'application
        QListWidgetItem* item = new QListWidgetItem(appName);
        // Stocker l'identifiant unique dans le rôle Qt::UserRole

        item->setData(Qt::UserRole, appId);
       

        if (isFavorite(m_mainWindow->userJsonArray, appId)) {

            QListWidgetItem* itemFav = new QListWidgetItem(appName);
            // Stocker l'identifiant unique dans le rôle Qt::UserRole

            itemFav->setData(Qt::UserRole, appId);

            ui->availableList->addItem(item);
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);


            ui->favoritesList->addItem(itemFav);
        }
        else {
            ui->availableList->addItem(item);
        }
    }

    ui->availableList->sortItems();
}

bool DialogAddFavorites::isFavorite(const QJsonArray& userJsonArray, int idItem) {

    for (const QJsonValue& value : userJsonArray) {

        
        int appId = value.toInt();
        qDebug() << "appId" << appId << "idItem" << idItem;
        if (appId == idItem) {
            return true;
        }
    }
    return false;
}

void DialogAddFavorites::populateFavoriteList(const QJsonArray& userJsonArray)
{
    ui->favoritesList->clear();  // On vide la liste avant de la remplir

    for (const QJsonValue& value : userJsonArray) {
        if (!value.isObject())
            continue;  // On ignore si ce n'est pas un objet

        QJsonObject obj = value.toObject();
        QString appName = obj.value("Name").toString();
        int appId = obj.value("ID").toInt();

        // Créer l'item avec le nom de l'application
        QListWidgetItem* item = new QListWidgetItem(appName);
        // Stocker l'identifiant unique dans le rôle Qt::UserRole
        item->setData(Qt::UserRole, appId);

        // Ajouter l'item dans favoritesList
        ui->favoritesList->addItem(item);
    }

    ui->favoritesList->sortItems();
}


DialogAddFavorites::~DialogAddFavorites()
{
    delete ui;
}

void DialogAddFavorites::on_addButton_clicked()
{
    // Même code pour l'ajout par bouton
    QList<QListWidgetItem*> selectedItems = ui->availableList->selectedItems();
    for (QListWidgetItem* item : selectedItems) {
        // Vérifier si l'élément n'est pas déjà dans la liste favorites
        QList<QListWidgetItem*> found = ui->favoritesList->findItems(item->text(), Qt::MatchExactly);
        if (found.isEmpty()) {
            QListWidgetItem* newItem = new QListWidgetItem(item->text());
            ui->favoritesList->addItem(newItem);
            // Désactiver l'item dans availableList
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        }
    }
}

void DialogAddFavorites::on_removeButton_clicked()
{
    // Retirer les éléments sélectionnés dans favoritesList
    QList<QListWidgetItem*> selectedItems = ui->favoritesList->selectedItems();
    for (QListWidgetItem* item : selectedItems) {
        QString text = item->text();
        int row = ui->favoritesList->row(item);
        QListWidgetItem* removedItem = ui->favoritesList->takeItem(row);
        delete removedItem;

        // Réactiver l'item correspondant dans availableList
        QList<QListWidgetItem*> availItems = ui->availableList->findItems(text, Qt::MatchExactly);
        for (QListWidgetItem* availItem : availItems) {
            availItem->setFlags(availItem->flags() | Qt::ItemIsEnabled);
        }
    }
}

QStringList DialogAddFavorites::selectedFavorites() const
{
    QStringList favorites;
    for (int i = 0; i < ui->favoritesList->count(); ++i) {
        favorites << ui->favoritesList->item(i)->text();
    }
    return favorites;
}

void DialogAddFavorites::onFavoritesRowsInserted(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent);
    // Planifie l'exécution à la fin de l'événement courant
    QTimer::singleShot(0, this, [=]() {
        // Partie existante : désactivation des items correspondants dans availableList
        for (int row = first; row <= last; row++) {
            QListWidgetItem* favItem = ui->favoritesList->item(row);
            if (favItem) {
                QString text = favItem->text();
                // Cherche dans availableList l'item correspondant
                QList<QListWidgetItem*> availItems = ui->availableList->findItems(text, Qt::MatchExactly);
                for (QListWidgetItem* availItem : availItems) {
                    // Si l'item est encore activé, le désactiver
                    if (availItem->flags() & Qt::ItemIsEnabled) {
                        availItem->setFlags(availItem->flags() & ~Qt::ItemIsEnabled);
                        qDebug() << "Désactivation de l'item" << availItem->text() << "dans availableList (drop)";
                    }
                }
            }
        }

        // Partie additionnelle : suppression des doublons dans favoritesList
        QSet<QString> seen;
        // Parcourir la liste des favoris en partant de la fin
        for (int i = ui->favoritesList->count() - 1; i >= 0; i--) {
            QListWidgetItem* item = ui->favoritesList->item(i);
            QString text = item->text();
            if (seen.contains(text)) {
                qDebug() << "Doublon détecté dans favoritesList, suppression:" << text;
                QListWidgetItem* removed = ui->favoritesList->takeItem(i);
                delete removed;
            }
            else {
                seen.insert(text);
            }
        }
        });
}

void DialogAddFavorites::onAvailableRowsInserted(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent);
    QTimer::singleShot(0, this, [=]() {
        // Créer un QMap pour regrouper tous les items par texte
        QMap<QString, QList<QListWidgetItem*>> groups;
        int total = ui->availableList->count();
        for (int i = 0; i < total; i++) {
            QListWidgetItem* item = ui->availableList->item(i);
            if (item)
                groups[item->text()].append(item);
        }

        // Parcourir chaque groupe d'items
        for (auto it = groups.begin(); it != groups.end(); ++it) {
            const QString& text = it.key();
            const QList<QListWidgetItem*>& items = it.value();
            if (items.size() > 1) { // Si plus d'un item a ce texte, il y a doublon(s)
                QListWidgetItem* original = nullptr;
                // Chercher l'item original désactivé (celui ajouté précédemment)
                for (QListWidgetItem* item : items) {
                    if (!(item->flags() & Qt::ItemIsEnabled)) {
                        original = item;
                        break;
                    }
                }
                // Si aucun original n'est trouvé, on considère le premier comme original
                if (!original) {
                    original = items.first();
                }
                // Réactiver l'item original
                original->setFlags(original->flags() | Qt::ItemIsEnabled);
                qDebug() << "Réactivation de l'item original:" << text;

                // Supprimer tous les autres items (duplicata)
                for (QListWidgetItem* item : items) {
                    if (item != original) {
                        int idx = ui->availableList->row(item);
                        QListWidgetItem* dup = ui->availableList->takeItem(idx);
                        delete dup;
                        qDebug() << "Duplication supprimée dans availableList:" << text;
                    }
                }

                // Supprimer l'item correspondant dans favoritesList, s'il existe
                QList<QListWidgetItem*> favItems = ui->favoritesList->findItems(text, Qt::MatchExactly);
                for (QListWidgetItem* favItem : favItems) {
                    int favIdx = ui->favoritesList->row(favItem);
                    QListWidgetItem* removedFav = ui->favoritesList->takeItem(favIdx);
                    delete removedFav;
                    qDebug() << "Item retiré de favoritesList:" << text;
                }
            }
        }
        });
}
