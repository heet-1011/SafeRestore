#include "../include/MainWindow.h"
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
    topBarLayout->addWidget(minBtn);

    QToolButton *closeBtn = new QToolButton();
    closeBtn->setIcon(QIcon("../assets/close.svg"));
    closeBtn->setIconSize(QSize(24, 24));
    closeBtn->setFixedSize(35, 35);
    closeBtn->setStyleSheet("QToolButton { color: white; border: none; border-radius: 3px; font-size: 16px; } "
                            "QToolButton:hover { background-color: #ff0000; }"
                            "QToolButton:pressed { background-color: #cc0000; }");
    topBarLayout->addWidget(closeBtn);
    mainLayout->addWidget(topBar);

    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(20, 20, 20, 20);
    contentLayout->setSpacing(0);

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

    headerRow->addWidget(instructionLabel);
    headerRow->addStretch();
    headerRow->addWidget(refreshBtn);
    contentLayout->addLayout(headerRow);

    QLabel *subHeader = new QLabel("Do not write or copy data to the affected drive to maximize recovery chances.");
    subHeader->setStyleSheet("font-size: 14px; color: #6b7280; margin-bottom: 20px;");
    contentLayout->addWidget(subHeader);

    QFrame *line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setStyleSheet("color: #e5e7eb;");
    contentLayout->addWidget(line);

    QLabel *builtInLabel = new QLabel("Built-in Drives / Partitions");
    builtInLabel->setStyleSheet("font-weight: bold; font-size: 18px; color: #111827; margin-top: 10px;");
    QScrollArea *scroll1 = new QScrollArea();
    scroll1->setFixedHeight(100);
    scroll1->setWidgetResizable(true);
    scroll1->setStyleSheet("QScrollArea { border: none; background: transparent; }"
                           "QScrollArea { border: none; background: transparent; }"

                           "QScrollBar:horizontal {"
                           "    border: none;"
                           "    background: transparent;"
                           "    height: 10px;"
                           "    margin: 0px 0px 0px 0px;"
                           "}"

                           "QScrollBar::handle:horizontal {"
                           "    background: #d1d5db;"
                           "    min-width: 30px;"
                           "    border-radius: 5px;"
                           "}"
                           "QScrollBar::handle:horizontal:hover {"
                           "    background: #9ca3af;"
                           "}"

                           "QScrollBar::add-line:horizontal {"
                           "    width: 0px;"
                           "}"
                           "QScrollBar::sub-line:horizontal {"
                           "    width: 0px;"
                           "}"

                           "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
                           "    background: none;"
                           "}");
    QWidget *scroll1Content = new QWidget();
    scroll1Content->setStyleSheet("background: transparent;");
    QHBoxLayout *hLayout1 = new QHBoxLayout(scroll1Content);
    hLayout1->setContentsMargins(0, 0, 0, 0);

    QLabel *externalLabel = new QLabel("External Drives / Partitions");
    externalLabel->setStyleSheet("font-weight: bold; font-size: 18px; color: #111827; margin-top: 10px;");
    QScrollArea *scroll2 = new QScrollArea();
    scroll2->setFixedHeight(90);
    scroll2->setWidgetResizable(true);
    scroll2->setStyleSheet("QScrollArea { border: none; background: transparent; }"
                           "QScrollArea { border: none; background: transparent; }"

                           "QScrollBar:horizontal {"
                           "    border: none;"
                           "    background: transparent;"
                           "    height: 10px;"
                           "    margin: 0px 0px 0px 0px;"
                           "}"

                           "QScrollBar::handle:horizontal {"
                           "    background: #d1d5db;"
                           "    min-width: 30px;"
                           "    border-radius: 5px;"
                           "}"
                           "QScrollBar::handle:horizontal:hover {"
                           "    background: #9ca3af;"
                           "}"

                           "QScrollBar::add-line:horizontal {"
                           "    width: 0px;"
                           "}"
                           "QScrollBar::sub-line:horizontal {"
                           "    width: 0px;"
                           "}"

                           "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
                           "    background: none;"
                           "}");
    QWidget *scroll2Content = new QWidget();
    scroll2Content->setStyleSheet("background: transparent;");
    QHBoxLayout *hLayout2 = new QHBoxLayout(scroll2Content);
    hLayout2->setContentsMargins(0, 0, 0, 0);

    vector<Utils::DriveInfo> activeDrives = Utils::listDrives();
    driveButtonGroup = new QButtonGroup(this);
    driveButtonGroup->setExclusive(true);

    for (const Utils::DriveInfo &drive : activeDrives)
    {
        double totalGB = drive.size / (1024.0 * 1024.0 * 1024.0);
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
        if (drive.isExternal)
        {
            hLayout2->addWidget(driveCard);
        }
        else
        {
            hLayout1->addWidget(driveCard);
        }
        driveButtonGroup->addButton(driveCard);
    }

    hLayout1->addStretch();
    hLayout2->addStretch();

    scroll1->setWidget(scroll1Content);
    scroll2->setWidget(scroll2Content);

    contentLayout->addWidget(builtInLabel);
    contentLayout->addWidget(scroll1);
    contentLayout->addWidget(externalLabel);
    contentLayout->addWidget(scroll2);
    contentLayout->addStretch();

    QHBoxLayout *bottomRow = new QHBoxLayout();
    QPushButton *nextBtn = new QPushButton("Next");
    nextBtn->setFixedSize(140, 45);
    nextBtn->setStyleSheet("QPushButton { background-color: #3b82f6; color: white; font-weight: bold; font-size: 18px; border-radius: 8px; }"
                           "QPushButton:hover { background-color: #2563eb; }"
                           "QPushButton:pressed { background-color: #1d4ed8; }");

    bottomRow->addStretch();
    bottomRow->addWidget(nextBtn);
    contentLayout->addLayout(bottomRow);

    mainLayout->addWidget(contentWidget);

    setCentralWidget(mainWidget);
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
    progressBar->setStyleSheet(
        "QProgressBar { border: 1px solid #e5e7eb; border-radius: 6px; background-color: #f3f4f6; }"
        "QProgressBar::chunk { background-color: #3b82f6; border-radius: 6px; }");
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