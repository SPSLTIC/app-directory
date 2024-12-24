#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QDir>
#include <QJsonArray>
#include <QCloseEvent>
#include <QMessageBox>
#include <QListWidgetItem>
#include "config.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QJsonArray jsonArray;
    QJsonArray userJsonArray;
    QJsonArray customJsonArray;

    bool loadData();
    bool initDirs();

    // Helper functions for paths
    QString getUserFilePath() const {
        return QDir(Config::USER_DATA_PATH).filePath("user.json");
    }
    QString getCustomEntriesPath() const {
        return QDir(Config::CUSTOM_DATA_PATH).filePath(Config::CUSTOM_FILENAME);
    }
    QString getCustomImagesPath() const {
        return QDir(Config::CUSTOM_DATA_PATH).filePath("custom_images");
    }

    enum class SortRole {
        Alphabetical,
        ID,
        Favorite,
        Type
    };

    struct SortCriteriaItem {
        QString name;
        SortRole role;
    };

    QVector<SortCriteriaItem> sortCriteria;
    SortRole currentSortRole;
    Qt::SortOrder currentSortOrder;
    void setupSortCriteria();
    QString getSortCriteriaString(SortRole role) const;
    bool compareItems(const QJsonObject &a, const QJsonObject &b) const;
    void populateList();

    QString getLatestBackupPath() const;
    void updateJsonArrays();
    void addCustomEntry(const QJsonObject &entry);
    void saveUserFile();
    void saveCustomEntriesFile();
    QString copyImageToStorage(const QString &originalImagePath);

    // Save system methods
    bool verifyJsonArray(const QString &filePath) const;
    bool createBackup() const;
    bool restoreFromBackup(const QString &backupFile);
    QString generateBackupFileName() const;
    bool ensureDirectoryExists(const QString &path) const;

    // Enhanced path management
    QString getBackupPath() const {
        return QDir(Config::USER_DATA_PATH).filePath("backups");
    }

    // Error handling
    void showSaveError(const QString &message) const {
        QMessageBox::warning(nullptr, tr("Error"), message);
    }

    void updateExistingItem(QListWidgetItem* item, QListWidgetItem* favItem,
                            const QJsonObject& obj);
    void createNewItem(const QJsonObject& obj);
    void updateUserJsonArray(int id, bool favorite);
    QSet<int> customEntriesMarkedForDeletion;
    void processMarkedForDeletionEntries();

private slots:
    void on_actionQuitter_triggered();
    void closeEvent(QCloseEvent *event) override;
    void on_actionApropos_triggered();
    void on_comboBox_currentIndexChanged(int index);
    void onSortOrderChanged();
    void on_lineEdit_textChanged(const QString &arg1);
    void clearSearch();
    void resetListVisibility();
    void on_pushButton_clicked();
    void on_actionParam_tres_triggered();
    void onFavoriteToggled(bool favorite);
    void updateFavoriteOrder(const QModelIndex &sourceIndex, int row);
};
#endif // MAINWINDOW_H
