#ifndef RICHITEM_H
#define RICHITEM_H

#include <QWidget>

namespace Ui {
class richitem;
}

class richitem : public QWidget
{
    Q_OBJECT

public:
    explicit richitem(
        const QString &path,
        const QString &text,
        const QString &imagePath,
        bool favorite,
        bool showFavoriteButton = true,
        QWidget *parent = nullptr);
    ~richitem();

    bool isFavorite() const { return m_favorite; }
    QString getText() const { return m_text; }
    QString getLabelText() const;
    void setFavorite(bool favorite);
    void updateContent(const QString& path, const QString& text,
                       const QString& imagePath, bool favorite);

signals:
    void favoriteToggled(bool favorite);

private:
    Ui::richitem *ui;
    bool m_favorite;
    bool m_showFavoriteButton;
    QString m_currentImagePath;
    QString m_text;
    void updateFavoriteButton();

private slots:
    void toggleFavorite();
    void handleLink(const QString &link);
};

#endif // RICHITEM_H
