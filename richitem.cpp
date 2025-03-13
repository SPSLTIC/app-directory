#include "richitem.h"
#include "ui_richitem.h"
#include "config.h"
#include "dialogadditem.h"
#include <QUrl>
#include <QDir>
#include <QProcess>
#include <QDesktopServices>
#include <QRegularExpression>
#include <QSettings>


richitem::richitem(
	MainWindow* mainWindow,
	const int id,
	const QString& path,
	const QString& text,
	const QString& imagepath,
	bool custom,
	bool favorite,
	bool showFavoriteButton,
	QWidget* parent) :
	QWidget(parent),
	id(id),
	path(path),
	ui(new Ui::richitem),
	m_favorite(favorite),
	m_showFavoriteButton(showFavoriteButton),
	m_text(text),
	m_mainWindow(mainWindow)
{
	ui->setupUi(this);

	setFixedHeight(50);

	updateContent(path, text, imagepath, custom, favorite);

	if (m_showFavoriteButton) {
		connect(ui->favButton, &QPushButton::clicked,
			this, &richitem::toggleFavorite);
	}
	else {
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
		}
		else {
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
	const QString& imagePath, bool custom, bool favorite) {
	m_text = text;
	m_favorite = favorite;

	ui->label_2->setText(text);
	ui->label_2->setTextFormat(Qt::RichText);
	ui->label_2->setOpenExternalLinks(false);

	ui->label_3->setHidden(true);

	ui->toolButton->setVisible(custom);
	ui->toolButton->setFixedHeight(30);
	ui->toolButton->setFixedWidth(30);
	
	if (!custom) {
		ui->label_2->setText("");
		//ui->label_2->
		ui->label_2->setHidden(false);
	}
	else {
		ui->label_2->setText("");
	}
	

	if (imagePath != m_currentImagePath) {
		m_currentImagePath = imagePath;
	}
	

	updateFavoriteButton();
}

void richitem::handleLink(const QString& link)
{
	qDebug() << "handleLink called with link:" << link;
	// Ouvrir le chemin via QProcess (ou QDesktopServices si souhaité)
	QProcess::startDetached("cmd.exe", { "/c", "start", "", link });
}

void richitem::mouseDoubleClickEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton) {
		handleLink(path);
	}
	QWidget::mouseDoubleClickEvent(event);
}

void richitem::setIconItemAsNew() {
	ui->label_3->setHidden(false);
	QPixmap scaledPixmap = ui->label_3->pixmap();
	
	ui->label_3->setPixmap(scaledPixmap.scaled(70, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void richitem::on_toolButton_clicked()
{
	QJsonArray customJsonArray = m_mainWindow->customJsonArray;
	QJsonObject currentEntry;

	bool found = false;

	for (const QJsonValue& value : customJsonArray) {
		if (value.isObject()) {
			QJsonObject obj = value.toObject();
			if (obj["ID"].toInt() == id) {
				currentEntry = obj;
				found = true;
				break;
			}
		}
	}

	if (!found) {
		qDebug() << "No custom entries found for" << id;
		return;
	}

	DialogAddItem* dialog = new DialogAddItem(currentEntry, this);

	connect(dialog, &DialogAddItem::customEntryUpdate, this, [this](const QJsonObject& updatedEntry) {
		qDebug() << "Modified entry received:" << updatedEntry;
		m_mainWindow->updateCustomEntry(updatedEntry);
		}, Qt::QueuedConnection);

	connect(dialog, &DialogAddItem::customEntryDeleted, this, [this](int id) {
		qDebug() << "Delete request received for ID:" << id;
		m_mainWindow->onCustomEntryDeleteRequested(id);
		});

	dialog->setAttribute(Qt::WA_DeleteOnClose);
	dialog->setFixedSize(dialog->size());
	dialog->exec();
}