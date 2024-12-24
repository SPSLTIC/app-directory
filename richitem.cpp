#include "richitem.h"
#include "ui_richitem.h"
#include "config.h"
#include <QUrl>
#include <QDir>
#include <QProcess>
#include <QDesktopServices>


richitem::richitem(
    const QString &path,
    const QString &text,
    const QString &imagepath,
    bool favorite,
    bool showFavoriteButton,
    QWidget *parent) :
    QWidget(parent),
    ui(new Ui::richitem),
    m_favorite(favorite),
    m_showFavoriteButton(showFavoriteButton),
    m_text(text)
{
    ui->setupUi(this);
    updateContent(path, text, imagepath, favorite);

    if (m_showFavoriteButton) {
        connect(ui->favButton, &QPushButton::clicked,
                this, &richitem::toggleFavorite);
    } else {
        ui->favButton->hide();
    }
}

richitem::~richitem()
{
    delete ui;
}

void richitem::toggleFavorite()
{
    if (m_showFavoriteButton) {
        m_favorite = !m_favorite;
        emit favoriteToggled(m_favorite);
        updateFavoriteButton();
        emit favoriteToggled(m_favorite);
    }
}

QString richitem::getLabelText() const
{
    return ui->label_2->text();
}

void richitem::updateFavoriteButton()
{
    if (m_showFavoriteButton) {
        ui->favButton->setText(m_favorite ? "★" : "☆");
        /*
         * show gold star on newly added to mark a difference.
         */
        // Only style the star color when it's a favorite
        if (m_favorite) {
            QPalette pal = ui->favButton->palette();
            pal.setColor(QPalette::ButtonText, QColor("#FFD700"));
            ui->favButton->setPalette(pal);
        } else {
            ui->favButton->setPalette(QPalette()); // use system colors
        }
    }
}

void richitem::setFavorite(bool favorite)
{
    if (m_favorite != favorite) {
        m_favorite = favorite;
        updateFavoriteButton();
    }
}

void richitem::updateContent(const QString& path, const QString& text,
                             const QString& imagePath, bool favorite) {
    m_text = text;
    m_favorite = favorite;

    QString linkText = QString("<a style='color: %1; text-decoration:none;' href='file:///%2'>%3</a>")
                           .arg("gray", path, text);

    ui->label_2->setText(linkText);
    ui->label_2->setTextFormat(Qt::RichText);
    ui->label_2->setOpenExternalLinks(false);

    if (imagePath != m_currentImagePath) {
        QPixmap pixmap;
        if (imagePath.startsWith(Config::USER_DATA_PATH)) {
            pixmap = QPixmap(imagePath);
        } else {
            QString cleanimagepath = QCoreApplication::applicationDirPath() + "/" + imagePath;
            pixmap = QPixmap(cleanimagepath);
        }

        if (!pixmap.isNull()) {
            pixmap = pixmap.scaled(65, 65, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            ui->label->setPixmap(pixmap);
        }
        m_currentImagePath = imagePath;
    }
    // Connect the link clicked signal
    connect(ui->label_2, &QLabel::linkActivated,
            this, &richitem::handleLink);
    updateFavoriteButton();
}

void richitem::handleLink(const QString &link) {
    // Remove 'file:///' if present
    QString actualPath = link;
    if (actualPath.startsWith("file:///")) {
        actualPath = actualPath.mid(8);
    }

    // Try using ShellExecute via QProcess
    QProcess::startDetached("cmd.exe", {"/c", "start", "", actualPath});
}
