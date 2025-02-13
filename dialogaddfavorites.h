#ifndef DIALOGADDFAVORITES_H
#define DIALOGADDFAVORITES_H

#include <QDialog>

#include "mainwindow.h"

namespace Ui {
    class DialogAddFavorites;
}

class DialogAddFavorites : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddFavorites(MainWindow* mainWindow, QWidget* parent = nullptr);
    ~DialogAddFavorites();

    // Accès aux listes favorites sélectionnées
    QStringList selectedFavorites() const;

private slots:
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void onFavoritesRowsInserted(const QModelIndex& parent, int first, int last);
    void onAvailableRowsInserted(const QModelIndex& parent, int first, int last);

private:
    Ui::DialogAddFavorites* ui;
    void populateAvailableList(const QJsonArray& jsonArray); 
    void populateFavoriteList(const QJsonArray& userJsonArray);
    bool isFavorite(const QJsonArray& userJsonArray, int idItem);
    MainWindow* m_mainWindow;

};



#endif // DIALOGADDFAVORITES_H
