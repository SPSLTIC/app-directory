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

    QJsonArray getSelectedFavorites() const;

private slots:
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void onFavoritesRowsInserted(const QModelIndex& parent, int first, int last);
    void onAvailableRowsInserted(const QModelIndex& parent, int first, int last);
    void on_availableSearch_textChanged(const QString& arg1);
    void on_favoritesSearch_textChanged(const QString& arg1);
    void onAvailableListItemPressed(QListWidgetItem* item);
    void onFavoritesListItemPressed(QListWidgetItem* item);

private:
    Ui::DialogAddFavorites* ui;
    void populateAvailableList(const QJsonArray& jsonArray); 
    void populateFavoriteList(const QJsonArray& jsonArray);
    bool isFavorite(const QJsonArray& userJsonArray, int idItem);
    void sortFavoritesByUserJsonArray(const QJsonArray& userJsonArray);
    MainWindow* m_mainWindow;

};



#endif // DIALOGADDFAVORITES_H
