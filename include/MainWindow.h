#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QPushButton>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QString>
#include <QButtonGroup>
#include "../include/Utils.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    private:
    QStackedWidget *stackedWidget;
    QString selectedSourcePath;
    QString selectedDestinationPath;
    QString recoveryFolderName = "saferestored_data";

    QPushButton *createDriveCard(const QString &title, const QString &storageStats, int usagePercent, const QString &iconPath);
    void loadDrives(QString excludePath, QHBoxLayout *hLayout1, QHBoxLayout *hLayout2, QButtonGroup *driveButtonGroup); 
    
    QHBoxLayout *hLayout1;
    QHBoxLayout *hLayout2;
    QButtonGroup *driveButtonGroup;

    QHBoxLayout *destHLayout1;
    QHBoxLayout *destHLayout2;
    QButtonGroup *destButtonGroup;
};

#endif