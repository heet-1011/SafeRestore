#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QPushButton>
#include <QPoint>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QString>
#include <QButtonGroup>
#include <QSplitter>
#include <QLabel>
#include <QTextEdit>
#include <QTreeView>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QListWidget>
#include <QSortFilterProxyModel>
#include "../include/BaseParser.h"
#include "../include/Utils.h"

class PathFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    PathFilterProxyModel(QObject *parent = nullptr) : QSortFilterProxyModel(parent) {}
    
    void setExcludedPath(const QString &path) {
        m_excludedPath = path;
        invalidateFilter();
    }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override {
        QFileSystemModel *model = qobject_cast<QFileSystemModel*>(sourceModel());
        if (!model || m_excludedPath.isEmpty()) return true;

        QModelIndex index = model->index(source_row, 0, source_parent);
        QString path = model->filePath(index);
        
        if (path == m_excludedPath) return false;
        return true;
    }

private:
    QString m_excludedPath;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:

    QWidget *topBar; 
    QPoint dragPosition;
    bool isDragging = false;

    QStackedWidget *stackedWidget;
    QString selectedSourcePath;
    uint64_t selectedSourceSize;
    QString selectedDestinationPath;
    QString recoveryFolderName = "saferestored_data";
    QString finalRecoveryPath;
    vector<RecoveredFileInfo> scannedFiles;
    vector<QWidget*> fileCardWidgets;

    QPushButton *createDriveCard(const QString &title, const QString &storageStats, int usagePercent, const QString &iconPath);
    void loadDrives(QString excludePath, QHBoxLayout *hLayout1, QHBoxLayout *hLayout2, QButtonGroup *driveButtonGroup);

    QHBoxLayout *hLayout1;
    QHBoxLayout *hLayout2;
    QButtonGroup *driveButtonGroup;


    QLayout *fileGridLayout;
    QSplitter *scanSplitter;

    QTreeView *destTreeView;
    QFileSystemModel *destModel;
    QSortFilterProxyModel *destFilter;
    QListWidget *sidebar;
    QLabel *destSelectedLabel;

    QWidget *bottomBar;
    QPushButton *prevBtn;
    QPushButton *nextBtn;
    QLineEdit *pathEdit;

    QWidget *previewPane;
    QLabel *previewFileNameLabel;
    QStackedWidget *previewStack;
    QLabel *fileInfoLabel;

    QLabel *imagePreviewLabel;
    QTextEdit *textPreviewArea;

    QWidget *createFileCard(const QString &fileName, const QString &fileType, const QString &filePath, uint32_t startCluster, uint32_t fileSize);
    void updatePreviewPane(const QString &filePath, const QString &fileName, const QString &ext, uint32_t startCluster, uint32_t fileSize);
    void showHexPreview(const QString &path);
    void relayoutFiles();
    void startScan();
};

#endif