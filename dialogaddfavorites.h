#ifndef DIALOGADDFAVORITES_H
#define DIALOGADDFAVORITES_H

#include <QDialog>

namespace Ui {
    class DialogAddFavorites;
}

class DialogAddFavorites : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAddFavorites(QWidget* parent = nullptr);
    ~DialogAddFavorites();

    // Accès aux listes favorites sélectionnées
    QStringList selectedFavorites() const;

private slots:
    void on_addButton_clicked();
    void on_removeButton_clicked();

private:
    Ui::DialogAddFavorites* ui;
};

#endif // DIALOGADDFAVORITES_H
