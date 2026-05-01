#include "../include/MainWindow.h"
#include "../include/ConfirmDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QSvgWidget>
#include <QScrollArea>
#include <QProgressBar>
#include <QPixmap>
#include <vector>
#include <QButtonGroup>
#include <QFrame>
#include <QLineEdit>
#include <QDir>
#include <QDialog>
using namespace std;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(900, 600);

    QWidget *mainWidget = new QWidget(this);
    mainWidget->setObjectName("mainContainer");
    mainWidget->setStyleSheet("QWidget#mainContainer { "
                              "background-color: #f9fafb; "
                              "border-radius: 10px; "
                              "}");
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QWidget *topBar = new QWidget(this);
    topBar->setFixedHeight(50);
    topBar->setObjectName("topBar");
    topBar->setStyleSheet("QWidget#topBar { background-color: #001833; border-top-left-radius: 10px; border-top-right-radius: 10px; }");

    QHBoxLayout *topBarLayout = new QHBoxLayout(topBar);
    topBarLayout->setContentsMargins(15, 0, 10, 0);

    QSvgWidget *logo = new QSvgWidget("../assets/safe-restore.svg", this);
    logo->setFixedSize(24, 24);
    topBarLayout->addWidget(logo);

    QLabel *titleLabel = new QLabel("SafeRestore", this);
    titleLabel->setStyleSheet("color: white; font-weight: bold; font-size: 16px; margin-left: 5px; margin-top:3px; ");
    topBarLayout->addWidget(titleLabel);

    topBarLayout->addStretch();

    QToolButton *minBtn = new QToolButton();
    minBtn->setIcon(QIcon("../assets/minimize.svg"));
    minBtn->setIconSize(QSize(24, 24));
    minBtn->setFixedSize(35, 35);
    minBtn->setStyleSheet("QToolButton { color: white; border: none; border-radius: 3px; font-size: 16px; } "
                          "QToolButton:hover { background-color: #1a4a66; }"
                          "QToolButton:pressed { background-color: #0f2e40; }");
    connect(minBtn, &QToolButton::clicked, this, [this]()
            {
    this->hide(); 
    this->showMinimized(); });
    topBarLayout->addWidget(minBtn);

    QToolButton *closeBtn = new QToolButton();
    closeBtn->setIcon(QIcon("../assets/close.svg"));
    closeBtn->setIconSize(QSize(24, 24));
    closeBtn->setFixedSize(35, 35);
    closeBtn->setStyleSheet("QToolButton { color: white; border: none; border-radius: 3px; font-size: 16px; } "
                            "QToolButton:hover { background-color: #ff0000; }"
                            "QToolButton:pressed { background-color: #cc0000; }");
    connect(closeBtn, &QToolButton::clicked, this, &MainWindow::close);
    topBarLayout->addWidget(closeBtn);
    mainLayout->addWidget(topBar);

    stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(stackedWidget);

    QWidget *sourcePage = new QWidget();
    QVBoxLayout *sourceLayout = new QVBoxLayout(sourcePage);
    sourceLayout->setContentsMargins(20, 20, 20, 20);
    sourceLayout->setSpacing(0);

    QHBoxLayout *headerRow = new QHBoxLayout();
    QLabel *instructionLabel = new QLabel("Select a target drive to begin the recovery scan.");
    instructionLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #1e1e2e;");

    QPushButton *refreshBtn = new QPushButton("Refresh Drives");
    refreshBtn->setIcon(QIcon("../assets/refresh.svg"));
    refreshBtn->setIconSize(QSize(20, 20));
    refreshBtn->setFixedSize(120, 35);
    refreshBtn->setStyleSheet("QPushButton { background-color: #f3f4f6; color: #1f2937; border-radius: 5px; border: 1px solid #d1d5db; }"
                              "QPushButton:hover { background-color: #e5e7eb; }"
                              "QPushButton:pressed { background-color: #d1d5db; }");
    connect(refreshBtn, &QPushButton::clicked, this, [this]()
            { loadDrives(NULL, hLayout1, hLayout2, driveButtonGroup); });
    headerRow->addWidget(instructionLabel);
    headerRow->addStretch();
    headerRow->addWidget(refreshBtn);
    sourceLayout->addLayout(headerRow);

    QLabel *subHeader = new QLabel("Do not write or copy data to the affected drive to maximize recovery chances.");
    subHeader->setStyleSheet("font-size: 14px; color: #6b7280; margin-bottom: 20px;");
    sourceLayout->addWidget(subHeader);

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #e5e7eb;");
    sourceLayout->addWidget(line);

    QString scrollStyle = "QScrollArea { border: none; background: transparent; }"
                          "QScrollBar:horizontal { border: none; background: transparent; height: 10px; }"
                          "QScrollBar::handle:horizontal { background: #d1d5db; min-width: 30px; border-radius: 5px; }"
                          "QScrollBar::handle:horizontal:hover { background: #9ca3af; }"
                          "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }"
                          "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: none; }";

    QLabel *builtInLabel = new QLabel("Built-in Drives / Partitions");
    builtInLabel->setStyleSheet("font-weight: bold; font-size: 18px; color: #111827; margin-top: 10px;");
    QScrollArea *scroll1 = new QScrollArea();
    scroll1->setFixedHeight(100);
    scroll1->setWidgetResizable(true);
    scroll1->setStyleSheet(scrollStyle);
    QWidget *scroll1Content = new QWidget();
    scroll1Content->setStyleSheet("background: transparent;");
    hLayout1 = new QHBoxLayout(scroll1Content);
    hLayout1->setContentsMargins(0, 0, 0, 0);

    QLabel *externalLabel = new QLabel("External Drives / Partitions");
    externalLabel->setStyleSheet("font-weight: bold; font-size: 18px; color: #111827; margin-top: 10px;");
    QScrollArea *scroll2 = new QScrollArea();
    scroll2->setFixedHeight(100);
    scroll2->setWidgetResizable(true);
    scroll2->setStyleSheet(scrollStyle);
    QWidget *scroll2Content = new QWidget();
    scroll2Content->setStyleSheet("background: transparent;");
    hLayout2 = new QHBoxLayout(scroll2Content);
    hLayout2->setContentsMargins(0, 0, 0, 0);

    scroll1->setWidget(scroll1Content);
    scroll2->setWidget(scroll2Content);

    sourceLayout->addWidget(builtInLabel);
    sourceLayout->addWidget(scroll1);
    sourceLayout->addWidget(externalLabel);
    sourceLayout->addWidget(scroll2);
    sourceLayout->addStretch();

    stackedWidget->addWidget(sourcePage);

    QWidget *destPage = new QWidget();
    QVBoxLayout *destLayout = new QVBoxLayout(destPage);
    destLayout->setContentsMargins(20, 20, 20, 20);
    destLayout->setSpacing(0);

    QHBoxLayout *destHeaderRow = new QHBoxLayout();
    QLabel *destInsLabel = new QLabel("Select a destination partition to save recovered files.");
    destInsLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #1e1e2e;");

    QPushButton *destRefreshBtn = new QPushButton("Refresh Drives");
    destRefreshBtn->setIcon(QIcon("../assets/refresh.svg"));
    destRefreshBtn->setIconSize(QSize(20, 20));
    destRefreshBtn->setFixedSize(120, 35);
    destRefreshBtn->setStyleSheet("QPushButton { background-color: #f3f4f6; color: #1f2937; border-radius: 5px; border: 1px solid #d1d5db; }"
                                  "QPushButton:hover { background-color: #e5e7eb; }"
                                  "QPushButton:pressed { background-color: #d1d5db; }");
    connect(destRefreshBtn, &QPushButton::clicked, this, [this]()
            { loadDrives(selectedSourcePath, destHLayout1, destHLayout2, destButtonGroup); });

    destHeaderRow->addWidget(destInsLabel);
    destHeaderRow->addStretch();
    destHeaderRow->addWidget(destRefreshBtn);
    destLayout->addLayout(destHeaderRow);

    QLabel *destSubHeader = new QLabel("Ensure the destination has enough space.");
    destSubHeader->setStyleSheet("font-size: 14px; color: #6b7280; margin-bottom: 20px;");
    destLayout->addWidget(destSubHeader);

    QFrame *line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setStyleSheet("color: #e5e7eb;");
    destLayout->addWidget(line2);

    QLabel *destBuiltInLabel = new QLabel("Available Built-in Drives / Partitions");
    destBuiltInLabel->setStyleSheet("font-weight: bold; font-size: 18px; color: #111827; margin-top: 10px;");
    QScrollArea *destScroll1 = new QScrollArea();
    destScroll1->setFixedHeight(100);
    destScroll1->setWidgetResizable(true);
    destScroll1->setStyleSheet(scrollStyle);
    QWidget *destScroll1Content = new QWidget();
    destScroll1Content->setStyleSheet("background: transparent;");
    destHLayout1 = new QHBoxLayout(destScroll1Content);
    destHLayout1->setContentsMargins(0, 0, 0, 0);

    QLabel *destExternalLabel = new QLabel("External Drives / Partitions");
    destExternalLabel->setStyleSheet("font-weight: bold; font-size: 18px; color: #111827; margin-top: 10px;");
    QScrollArea *destScroll2 = new QScrollArea();
    destScroll2->setFixedHeight(100);
    destScroll2->setWidgetResizable(true);
    destScroll2->setStyleSheet(scrollStyle);
    QWidget *destScroll2Content = new QWidget();
    destScroll2Content->setStyleSheet("background: transparent;");
    destHLayout2 = new QHBoxLayout(destScroll2Content);
    destHLayout2->setContentsMargins(0, 0, 0, 0);

    destScroll1->setWidget(destScroll1Content);
    destScroll2->setWidget(destScroll2Content);

    destLayout->addWidget(destBuiltInLabel);
    destLayout->addWidget(destScroll1);
    destLayout->addWidget(destExternalLabel);
    destLayout->addWidget(destScroll2);
    destLayout->addStretch();

    stackedWidget->addWidget(destPage);

    QWidget *scanPage = new QWidget();
    QVBoxLayout *scanLayout = new QVBoxLayout(scanPage);
    QLabel *scanLabel = new QLabel("Scanning in progress...");
    scanLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #1e1e2e;");
    scanLayout->addWidget(scanLabel);
    scanLayout->addStretch();

    stackedWidget->addWidget(scanPage);

    QWidget *bottomBar = new QWidget(this);
    bottomBar->setFixedHeight(70);
    bottomBar->setStyleSheet("background-color: #f9fafb; border-top: 1px solid #e5e7eb;");

    QHBoxLayout *bottomRow = new QHBoxLayout(bottomBar);
    bottomRow->setContentsMargins(20, 0, 20, 0);

    QPushButton *prevBtn = new QPushButton("Previous");
    prevBtn->setFixedSize(140, 45);
    prevBtn->setStyleSheet("QPushButton { background-color: #3b82f6; color: white; font-weight: bold; font-size: 18px; border-radius: 8px; }"
                           "QPushButton:hover { background-color: #2563eb; }"
                           "QPushButton:pressed { background-color: #1d4ed8; }");
    prevBtn->setVisible(false);
    QPushButton *nextBtn = new QPushButton("Next");
    nextBtn->setFixedSize(140, 45);
    nextBtn->setStyleSheet("QPushButton { background-color: #3b82f6; color: white; font-weight: bold; font-size: 18px; border-radius: 8px; }"
                           "QPushButton:hover { background-color: #2563eb; }"
                           "QPushButton:pressed { background-color: #1d4ed8; }");

    QLineEdit *pathEdit = new QLineEdit();
    pathEdit->setPlaceholderText("Enter folder name (default: saferestored_data)");
    pathEdit->setFixedWidth(400);
    pathEdit->setFixedHeight(45);
    pathEdit->setStyleSheet(
        "QLineEdit {"
        "   border: 2px solid #e5e7eb;"
        "   border-radius: 8px;"
        "   padding: 0 10px;"
        "   background: white;"
        "   font-size: 16px;"
        "   selection-background-color: #3b82f6;"
        "}"
        "QLineEdit:focus {"
        "   border: 2px solid #3b82f6;"
        "}");
    pathEdit->setVisible(false);

    bottomRow->addWidget(prevBtn);
    bottomRow->addStretch();
    bottomRow->addWidget(pathEdit);
    bottomRow->addStretch();
    bottomRow->addWidget(nextBtn);
    mainLayout->addWidget(stackedWidget);
    mainLayout->addWidget(bottomBar);
    setCentralWidget(mainWidget);

    driveButtonGroup = new QButtonGroup(this);
    driveButtonGroup->setExclusive(true);
    destButtonGroup = new QButtonGroup(this);
    destButtonGroup->setExclusive(true);

    connect(nextBtn, &QPushButton::clicked, this, [this, nextBtn, prevBtn, pathEdit]()
            {
        int currentIndex = stackedWidget->currentIndex();
        if (currentIndex == 0)
        {
            QAbstractButton *selected = driveButtonGroup->checkedButton();
            if (selected)
            {
                selectedSourcePath = selected->property("drivePath").toString();
                loadDrives(selectedSourcePath, destHLayout1, destHLayout2, destButtonGroup);
                stackedWidget->setCurrentIndex(1);
                nextBtn->setText("Start Scan");
                prevBtn->setVisible(true);
                pathEdit->setVisible(true);
            }
        }
        else if (currentIndex == 1)
        {
            QAbstractButton *destSelected = destButtonGroup->checkedButton();
            if (destSelected)
            {
                selectedDestinationPath = destSelected->property("drivePath").toString();
                QString editText = pathEdit->text().trimmed();
                if (!editText.isEmpty())
                {
                    recoveryFolderName = editText;
                }
                else
                {
                    recoveryFolderName = "saferestored_data";
                }
                QString fullDestinationPath = QDir(selectedDestinationPath).filePath(recoveryFolderName);

                QString confirmMsg = QString(
                "<p>You are about to start the recovery process with the following settings:</p>"
                "<p style='margin-left:10px;'>"
                "<b>• Source:</b> <span style='color:#3b82f6;'>%1</span><br>"
                "<b>• Destination:</b> <span style='color:#3b82f6;'>%2</span><br>"
                "</p>"
                "<p>This may take a while depending on the drive size.</p>"
                ).arg(selectedSourcePath, fullDestinationPath);

                ConfirmDialog diag("Confirm Recovery", confirmMsg, this);
                        if (diag.exec() == QDialog::Accepted)
                        {
                            stackedWidget->setCurrentIndex(2);
                            pathEdit->setVisible(false);
                            nextBtn->setText("Recover");
                        }
                    }
                } });

    connect(prevBtn, &QPushButton::clicked, this, [this, prevBtn, nextBtn, pathEdit]()
            {
        int currentIndex = stackedWidget->currentIndex();

        if (currentIndex == 1) {
                stackedWidget->setCurrentIndex(0);
                nextBtn->setText("Next");
                prevBtn->setVisible(false);
                pathEdit->setVisible(false);
        } else if(currentIndex == 2)  {
                stackedWidget->setCurrentIndex(1);
                pathEdit->setVisible(true);
                nextBtn->setText("Next");
        } });

    loadDrives(NULL, hLayout1, hLayout2, driveButtonGroup);
}

void MainWindow::loadDrives(QString excludePath, QHBoxLayout *hLayout1, QHBoxLayout *hLayout2, QButtonGroup *driveButtonGroup)
{
    QLayoutItem *item;
    while ((item = hLayout1->takeAt(0)) != nullptr)
    {
        if (item->widget())
            delete item->widget();
        delete item;
    }
    while ((item = hLayout2->takeAt(0)) != nullptr)
    {
        if (item->widget())
            delete item->widget();
        delete item;
    }
    for (QAbstractButton *btn : driveButtonGroup->buttons())
    {
        driveButtonGroup->removeButton(btn);
    }

    vector<Utils::DriveInfo> activeDrives = Utils::listDrives();

    for (const Utils::DriveInfo &drive : activeDrives)
    {
        double totalGB = drive.size / (1024.0 * 1024.0 * 1024.0);
        if (QString::fromStdString(drive.path) == excludePath)
            continue;
        QString title = QString::fromStdString(drive.name + " (" + drive.path + ")");
        QString stats;
        int percent = -1;

        if (drive.usedSize > 0)
        {
            double usedGB = drive.usedSize / (1024.0 * 1024.0 * 1024.0);
            double freeGB = totalGB - usedGB;
            percent = (drive.usedSize * 100) / drive.size;
            stats = QString::asprintf("%.1f GB free of %.1f GB", freeGB, totalGB);
        }
        else
        {
            stats = QString::asprintf("Raw/Unmounted • %.1f GB", totalGB);
        }

        QPushButton *driveCard = createDriveCard(title, stats, percent, "../assets/drive.svg");
        driveCard->setProperty("drivePath", QString::fromStdString(drive.path));
        if (drive.isExternal)
            hLayout2->addWidget(driveCard);
        else
            hLayout1->addWidget(driveCard);

        driveButtonGroup->addButton(driveCard);
    }

    hLayout1->addStretch();
    hLayout2->addStretch();
}

QPushButton *MainWindow::createDriveCard(const QString &title, const QString &storageStats, int usagePercent, const QString &iconPath)
{
    QPushButton *card = new QPushButton();
    card->setFixedSize(260, 85);
    card->setCheckable(true);
    card->setCursor(Qt::PointingHandCursor);
    card->setStyleSheet(
        "QPushButton {"
        "   background-color: white;"
        "   border: 1px solid #e5e7eb;"
        "   border-radius: 8px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #f9fafb;"
        "   border: 1px solid #bfdbfe;"
        "}"
        "QPushButton:checked {"
        "   background-color: #eff6ff;"
        "   border: 2px solid #3b82f6;"
        "}");

    QHBoxLayout *mainCardLayout = new QHBoxLayout(card);
    mainCardLayout->setContentsMargins(12, 12, 12, 12);
    mainCardLayout->setSpacing(15);

    QLabel *iconLabel = new QLabel();
    iconLabel->setFixedSize(45, 45);
    iconLabel->setStyleSheet("border: none; background: transparent;");
    iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    if (!iconPath.isEmpty())
    {
        iconLabel->setPixmap(QIcon(iconPath).pixmap(45, 45));
    }
    else
    {
        iconLabel->setStyleSheet("background-color: #e5e7eb; border-radius: 5px;");
    }
    mainCardLayout->addWidget(iconLabel);

    QVBoxLayout *rightLayout = new QVBoxLayout();
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(4);

    QLabel *titleLabel = new QLabel(title);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 13px; color: #1f2937; border: none; background: transparent;");
    titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    QProgressBar *progressBar = new QProgressBar();
    progressBar->setFixedHeight(12);
    progressBar->setValue(usagePercent);
    progressBar->setTextVisible(false);
    progressBar->setStyleSheet("QProgressBar {"
                               "   border: 1px solid #e5e7eb;"
                               "   border-radius: 6px;"
                               "   background-color: #f3f4f6;"
                               "   text-align: center;"
                               "}"
                               "QProgressBar::chunk {"
                               "   background-color: #3b82f6;"
                               "   border-radius: 5px;"
                               "}");
    progressBar->setAttribute(Qt::WA_TransparentForMouseEvents);

    QLabel *storageStatsLabel = new QLabel(storageStats);
    storageStatsLabel->setStyleSheet("color: #6b7280; font-size: 11px; border: none; background: transparent;");
    storageStatsLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    rightLayout->addWidget(titleLabel);
    rightLayout->addWidget(progressBar);
    rightLayout->addWidget(storageStatsLabel);
    rightLayout->addStretch();

    mainCardLayout->addLayout(rightLayout);
    return card;
}

MainWindow::~MainWindow()
{
}
#include "moc_MainWindow.cpp"