#include "DialogAddFavorites.h"
#include "ui_DialogAddFavorites.h"

DialogAddFavorites::DialogAddFavorites(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::DialogAddFavorites)
{
    ui->setupUi(this);

    // Remplir la liste des applications disponibles (exemple statique)
    ui->availableList->addItems(QStringList() << "Application A" << "Application B" << "Application C");
    // Optionnel : activer le glisser-d�poser sur les QListWidgets
    ui->availableList->setDragEnabled(true);
    ui->availableList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->favoritesList->setAcceptDrops(true);
    ui->favoritesList->setDropIndicatorShown(true);
    ui->favoritesList->setDragDropMode(QAbstractItemView::InternalMove);
}

DialogAddFavorites::~DialogAddFavorites()
{
    delete ui;
}

void DialogAddFavorites::on_addButton_clicked()
{
    // R�cup�rer les �l�ments s�lectionn�s dans la liste disponible
    QList<QListWidgetItem*> selectedItems = ui->availableList->selectedItems();
    for (QListWidgetItem* item : selectedItems) {
        // V�rifier si l'�l�ment n'est pas d�j� dans la liste favorites
        QList<QListWidgetItem*> found = ui->favoritesList->findItems(item->text(), Qt::MatchExactly);
        if (found.isEmpty()) {
            // Cloner l'item et l'ajouter � la liste des favoris
            QListWidgetItem* newItem = new QListWidgetItem(item->text());
            ui->favoritesList->addItem(newItem);
        }
    }
}

void DialogAddFavorites::on_removeButton_clicked()
{
    // R�cup�rer les �l�ments s�lectionn�s dans la liste des favoris
    QList<QListWidgetItem*> selectedItems = ui->favoritesList->selectedItems();
    qDeleteAll(selectedItems); // Supprime les items s�lectionn�s
}

QStringList DialogAddFavorites::selectedFavorites() const
{
    QStringList favorites;
    for (int i = 0; i < ui->favoritesList->count(); ++i) {
        favorites << ui->favoritesList->item(i)->text();
    }
    return favorites;
}
