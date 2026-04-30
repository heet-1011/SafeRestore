#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QPushButton>
#include <QButtonGroup>
#include "../include/Utils.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    private:
    QPushButton *createDriveCard(const QString &title, const QString &storageStats, int usagePercent, const QString &iconPath);
    QButtonGroup *driveButtonGroup;
};

#endif