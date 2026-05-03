#include "../include/MainWindow.h"
#include "../include/ConfirmDialog.h"
#include "../include/DriveReader.h"
#include "../include/Fat32Parser.h"
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
#include <QTextEdit>
#include <QDir>
#include <QDialog>
#include <QCheckBox>
#include <QSplitter>
#include <QDebug>
#include <QApplication>
#include <QMessageBox>
#include <QLayout>
#include <QStyle>
#include <QSizePolicy>
#include <QFileDialog>

class FlowLayout : public QLayout
{
public:
    explicit FlowLayout(QWidget *parent, int margin = -1, int hSpacing = -1, int vSpacing = -1)
        : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
    {
        setContentsMargins(margin, margin, margin, margin);
    }
    explicit FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1)
        : m_hSpace(hSpacing), m_vSpace(vSpacing)
    {
        setContentsMargins(margin, margin, margin, margin);
    }
    ~FlowLayout()
    {
        QLayoutItem *item;
        while ((item = takeAt(0)))
            delete item;
    }

    void addItem(QLayoutItem *item) override { itemList.append(item); }
    int horizontalSpacing() const { return m_hSpace >= 0 ? m_hSpace : smartSpacing(QStyle::PM_LayoutHorizontalSpacing); }
    int verticalSpacing() const { return m_vSpace >= 0 ? m_vSpace : smartSpacing(QStyle::PM_LayoutVerticalSpacing); }
    Qt::Orientations expandingDirections() const override { return {}; }
    bool hasHeightForWidth() const override { return true; }
    int heightForWidth(int width) const override { return doLayout(QRect(0, 0, width, 0), true); }
    int count() const override { return itemList.size(); }
    QLayoutItem *itemAt(int index) const override { return itemList.value(index); }
    QSize minimumSize() const override
    {
        QSize size;
        for (const QLayoutItem *item : itemList)
            size = size.expandedTo(item->minimumSize());
        const QMargins margins = contentsMargins();
        size += QSize(margins.left() + margins.right(), margins.top() + margins.bottom());
        return size;
    }
    void setGeometry(const QRect &rect) override
    {
        QLayout::setGeometry(rect);
        doLayout(rect, false);
    }
    QSize sizeHint() const override { return minimumSize(); }
    QLayoutItem *takeAt(int index) override
    {
        if (index >= 0 && index < itemList.size())
            return itemList.takeAt(index);
        return nullptr;
    }

private:
    int doLayout(const QRect &rect, bool testOnly) const
    {
        int left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);
        QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
        int x = effectiveRect.x();
        int y = effectiveRect.y();
        int lineHeight = 0;

        QList<QList<QLayoutItem *>> rows;
        QList<QLayoutItem *> currentRow;
        int currentRowWidth = 0;

        for (QLayoutItem *item : itemList)
        {
            int spaceX = horizontalSpacing();
            int nextX = x + item->sizeHint().width() + spaceX;
            if (nextX - spaceX > effectiveRect.right() && !currentRow.isEmpty())
            {
                rows.append(currentRow);
                currentRow.clear();
                x = effectiveRect.x();
                nextX = x + item->sizeHint().width() + spaceX;
            }
            currentRow.append(item);
            x = nextX;
        }
        if (!currentRow.isEmpty())
            rows.append(currentRow);

        int currentY = effectiveRect.y();
        for (const QList<QLayoutItem *> &row : rows)
        {
            int rowWidth = 0;
            int rowHeight = 0;
            for (QLayoutItem *item : row)
            {
                rowWidth += item->sizeHint().width();
                rowHeight = qMax(rowHeight, item->sizeHint().height());
            }
            rowWidth += (row.size() - 1) * horizontalSpacing();

            int startX = effectiveRect.x() + (effectiveRect.width() - rowWidth) / 2;
            int currentX = startX;

            for (QLayoutItem *item : row)
            {
                if (!testOnly)
                    item->setGeometry(QRect(QPoint(currentX, currentY), item->sizeHint()));
                currentX += item->sizeHint().width() + horizontalSpacing();
            }
            currentY += rowHeight + verticalSpacing();
            lineHeight = rowHeight;
        }

        return currentY - rect.y() + bottom;
    }
    int smartSpacing(QStyle::PixelMetric pm) const
    {
        QObject *parent = this->parent();
        if (!parent)
            return -1;
        if (parent->isWidgetType())
        {
            QWidget *pw = static_cast<QWidget *>(parent);
            return pw->style()->pixelMetric(pm, nullptr, pw);
        }
        return static_cast<QLayout *>(parent)->spacing();
    }
    QList<QLayoutItem *> itemList;
    int m_hSpace, m_vSpace;
};

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

    topBar = new QWidget(this);
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

    bottomBar = new QWidget(this);
    bottomBar->setFixedHeight(70);
    bottomBar->setStyleSheet("background-color: #f9fafb; border-top: 1px solid #e5e7eb; border-bottom-left-radius: 10px; border-bottom-right-radius: 10px; ");

    QHBoxLayout *bottomRow = new QHBoxLayout(bottomBar);
    bottomRow->setContentsMargins(20, 0, 20, 0);

    prevBtn = new QPushButton("Previous");
    prevBtn->setFixedSize(140, 45);
    prevBtn->setStyleSheet("QPushButton { background-color: #3b82f6; color: white; font-weight: bold; font-size: 18px; border-radius: 8px; }"
                           "QPushButton:hover { background-color: #2563eb; }"
                           "QPushButton:pressed { background-color: #1d4ed8; }");
    prevBtn->setVisible(false);

    nextBtn = new QPushButton("Next");
    nextBtn->setEnabled(false);
    nextBtn->setFixedSize(140, 45);
    nextBtn->setStyleSheet("QPushButton { background-color: #3b82f6; color: white; font-weight: bold; font-size: 18px; border-radius: 8px; }"
                           "QPushButton:hover { background-color: #2563eb; }"
                           "QPushButton:pressed { background-color: #1d4ed8; }");

    pathEdit = new QLineEdit();
    pathEdit->setPlaceholderText("Enter folder name (default: saferestored_data)");
    pathEdit->setFixedWidth(300);
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

    QString globalScrollStyle =
        "QScrollArea { border: none; background: transparent; }"
        "QScrollBar:horizontal { border: none; background: transparent; height: 8px; margin: 0px; }"
        "QScrollBar::handle:horizontal { background: #d1d5db; min-width: 20px; border-radius: 4px; }"
        "QScrollBar::handle:horizontal:hover { background: #9ca3af; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; height: 0px; }"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: none; }"
        "QScrollBar:vertical { border: none; background: transparent; width: 8px; margin: 0px; }"
        "QScrollBar::handle:vertical { background: #d1d5db; min-height: 20px; border-radius: 4px; }"
        "QScrollBar::handle:vertical:hover { background: #9ca3af; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { width: 0px; height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }";

    this->setStyleSheet(this->styleSheet() + globalScrollStyle);

    QLabel *builtInLabel = new QLabel("Built-in Drives / Partitions");
    builtInLabel->setStyleSheet("font-weight: bold; font-size: 18px; color: #111827; margin-top: 10px;");
    QScrollArea *scroll1 = new QScrollArea();
    scroll1->setFixedHeight(100);
    scroll1->setWidgetResizable(true);
    scroll1->setStyleSheet(globalScrollStyle);
    QWidget *scroll1Content = new QWidget();
    scroll1Content->setStyleSheet("background: transparent;");
    hLayout1 = new QHBoxLayout(scroll1Content);
    hLayout1->setContentsMargins(0, 0, 0, 0);

    QLabel *externalLabel = new QLabel("External Drives / Partitions");
    externalLabel->setStyleSheet("font-weight: bold; font-size: 18px; color: #111827; margin-top: 10px;");
    QScrollArea *scroll2 = new QScrollArea();
    scroll2->setFixedHeight(100);
    scroll2->setWidgetResizable(true);
    scroll2->setStyleSheet(globalScrollStyle);
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

    destHeaderRow->addWidget(destInsLabel);
    destHeaderRow->addStretch();
    destLayout->addLayout(destHeaderRow);

    QLabel *destSubHeader = new QLabel("Navigate and select the folder where you want to save recovered files.");
    destSubHeader->setStyleSheet("font-size: 14px; color: #6b7280; margin-bottom: 20px;");
    destLayout->addWidget(destSubHeader);

    QFrame *line2 = new QFrame();
    line2->setFrameShape(QFrame::HLine);
    line2->setStyleSheet("color: #e5e7eb;");
    destLayout->addWidget(line2);

    QHBoxLayout *fileManagerLayout = new QHBoxLayout();
    fileManagerLayout->setSpacing(10);

    sidebar = new QListWidget();
    sidebar->setFixedWidth(220);
    sidebar->setFrameShape(QFrame::NoFrame);
    sidebar->setSpacing(4);
    sidebar->setStyleSheet(
        "QListWidget {"
        "   background-color: #f9fafb;"
        "   border-right: 1px solid #e5e7eb;"
        "   outline: none;"
        "   padding: 10px;"
        "}"
        "QListWidget::item {"
        "   padding: 12px 15px;"
        "   border-radius: 10px;"
        "   color: #4b5563;"
        "   font-weight: 500;"
        "   font-size: 14px;"
        "   margin-bottom: 2px;"
        "}"
        "QListWidget::item:hover {"
        "   background-color: #f3f4f6;"
        "   color: #111827;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #3b82f6;"
        "   color: white;"
        "}"
    );

    destModel = new QFileSystemModel(this);
    destModel->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Drives | QDir::AllEntries | QDir::System | QDir::Hidden);
    destModel->setRootPath("/");

    destFilter = new PathFilterProxyModel(this);
    destFilter->setSourceModel(destModel);

    destTreeView = new QTreeView();
    destTreeView->setModel(destFilter);
    destTreeView->setFrameShape(QFrame::NoFrame);
    destTreeView->setRootIndex(destFilter->mapFromSource(destModel->index(QDir::homePath()))); 
    destTreeView->setColumnHidden(1, true);
    destTreeView->setColumnHidden(2, true);
    destTreeView->setColumnHidden(3, true);
    destTreeView->header()->setStretchLastSection(true);
    destTreeView->header()->setStyleSheet(
        "QHeaderView::section {"
        "   background-color: #f9fafb;"
        "   padding: 10px;"
        "   border: none;"
        "   border-bottom: 1px solid #e5e7eb;"
        "   color: #6b7280;"
        "   font-weight: bold;"
        "   text-transform: uppercase;"
        "   font-size: 11px;"
        "}"
    );
    destTreeView->setAnimated(true);
    destTreeView->setIndentation(25);
    destTreeView->setSortingEnabled(true);
    destTreeView->sortByColumn(0, Qt::AscendingOrder);
    destTreeView->setStyleSheet(
        "QTreeView {"
        "   background-color: white;"
        "   outline: none;"
        "   font-size: 14px;"
        "   padding-top: 5px;"
        "}"
        "QTreeView::item {"
        "   padding: 10px;"
        "   color: #1f2937;"
        "   border-bottom: 1px solid #f3f4f6;"
        "}"
        "QTreeView::item:hover {"
        "   background-color: #f9fafb;"
        "}"
        "QTreeView::item:selected {"
        "   background-color: #eff6ff;"
        "   color: #3b82f6;"
        "   font-weight: 600;"
        "}"
        "QTreeView::branch {"
        "   background-color: transparent;"
        "}"
        "QTreeView::branch:has-children:!has-siblings:closed,"
        "QTreeView::branch:closed:has-children:has-siblings {"
        "   image: url(../assets/chevron-right.svg);"
        "}"
        "QTreeView::branch:open:has-children:!has-siblings,"
        "QTreeView::branch:open:has-children:has-siblings {"
        "   image: url(../assets/chevron-down.svg);"
        "}"
    );

    fileManagerLayout->addWidget(sidebar);
    fileManagerLayout->addWidget(destTreeView, 1);

    QFrame *managerContainer = new QFrame();
    managerContainer->setLayout(fileManagerLayout);
    managerContainer->setStyleSheet("QFrame { background-color: white; border: 1px solid #e5e7eb; border-radius: 12px; overflow: hidden; }");
    destLayout->addWidget(managerContainer, 1);
    
    destSelectedLabel = new QLabel("Selected Destination Base: " + QDir::homePath());
    destSelectedLabel->setStyleSheet("font-size: 12px; color: #6b7280; margin-top: 5px;");
    destLayout->addWidget(destSelectedLabel);

    connect(sidebar, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        QString text = item->text();
        QModelIndex sourceIdx;
        if (text == "This PC") {
            sourceIdx = destModel->index("/media/" + QString::fromLocal8Bit(getenv("USER")));
        } else if (text == "Home") {
            sourceIdx = destModel->index(QDir::homePath());
        } else if (text == "System Root (/)") {
            sourceIdx = destModel->index("/");
        } else {
            QString path = item->data(Qt::UserRole).toString();
            if (!path.isEmpty()) {
                sourceIdx = destModel->index(path);
            }
        }
        if (sourceIdx.isValid()) {
            destTreeView->setRootIndex(destFilter->mapFromSource(sourceIdx));
        }
    });

    connect(destTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection &selected, const QItemSelection &deselected) {
        if (!selected.isEmpty()) {
            QModelIndex proxyIndex = destTreeView->selectionModel()->currentIndex();
            QModelIndex sourceIndex = destFilter->mapToSource(proxyIndex);
            QString path = destModel->filePath(sourceIndex);
            QFileInfo fi(path);
            QString displayPath = fi.isDir() ? path : fi.absolutePath();
            destSelectedLabel->setText("Selected Destination Base: " + displayPath);
            nextBtn->setEnabled(true);
        }
    });

    stackedWidget->addWidget(destPage);

    QWidget *scanPage = new QWidget();
    QVBoxLayout *scanLayout = new QVBoxLayout(scanPage);
    scanLayout->setContentsMargins(20, 20, 20, 20);
    scanLayout->setSpacing(0);

    QHBoxLayout *scanHeaderRow = new QHBoxLayout();
    QLabel *scanLabel = new QLabel("Scan Result");
    scanLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #1e1e2e;");

    QPushButton *scanRefreshBtn = new QPushButton(" Rescan Drive");
    scanRefreshBtn->setIcon(QIcon("../assets/refresh.svg"));
    scanRefreshBtn->setFixedSize(140, 35);
    scanRefreshBtn->setStyleSheet("QPushButton { background-color: #f3f4f6; color: #1f2937; border-radius: 5px; border: 1px solid #d1d5db; font-weight: bold; }"
                                  "QPushButton:hover { background-color: #e5e7eb; }"
                                  "QPushButton:pressed { background-color: #d1d5db; }");
    connect(scanRefreshBtn, &QPushButton::clicked, this, &MainWindow::startScan);

    scanHeaderRow->addWidget(scanLabel);
    scanHeaderRow->addStretch();
    scanHeaderRow->addWidget(scanRefreshBtn);
    scanLayout->addLayout(scanHeaderRow);

    QLabel *scanSubHeader = new QLabel("Deleted files to be recover.");
    scanSubHeader->setStyleSheet("font-size: 14px; color: #6b7280; margin-bottom: 20px;");
    scanLayout->addWidget(scanSubHeader);

    QFrame *line3 = new QFrame();
    line3->setFrameShape(QFrame::HLine);
    line3->setStyleSheet("color: #e5e7eb;");
    scanLayout->addWidget(line3);

    scanSplitter = new QSplitter(Qt::Horizontal, scanPage);
    scanSplitter->setHandleWidth(1);
    scanSplitter->setStyleSheet("QSplitter::handle { background-color: #e5e7eb; }");
    scanSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scanLayout->addWidget(scanSplitter, 1);

    QScrollArea *fileGridScroll = new QScrollArea();
    fileGridScroll->setWidgetResizable(true);
    fileGridScroll->setStyleSheet("QScrollArea { border: none; background: transparent; }");

    QWidget *gridContainer = new QWidget();
    gridContainer->setStyleSheet("background: transparent;");
    fileGridLayout = new FlowLayout(gridContainer, 15, 15, 15);
    gridContainer->setLayout(fileGridLayout);
    fileGridScroll->setWidget(gridContainer);

    previewPane = new QWidget();
    previewPane->setMinimumWidth(500);
    previewPane->setMaximumWidth(500);
    previewPane->setVisible(false);
    previewPane->setStyleSheet("background-color: white; border: none;");

    QVBoxLayout *pLayout = new QVBoxLayout(previewPane);

    QHBoxLayout *pHeader = new QHBoxLayout();
    previewFileNameLabel = new QLabel("File Preview");
    previewFileNameLabel->setStyleSheet("font-size: 16px; font-weight: bold; color: #001833; border: none;");
    QToolButton *closePBtn = new QToolButton();
    closePBtn->setFixedSize(30, 30);
    closePBtn->setIcon(QIcon("../assets/close.svg"));
    closePBtn->setStyleSheet("QToolButton { background-color: #303030; color: white; border: none; border-radius: 3px; font-size: 16px; } "
                             "QToolButton:hover { background-color: #000000; }"
                             "QToolButton:pressed { background-color: #333333; }");
    pHeader->addWidget(previewFileNameLabel);
    pHeader->addStretch();
    pHeader->addWidget(closePBtn);
    pLayout->addLayout(pHeader);

    previewStack = new QStackedWidget();

    QLabel *noPrev = new QLabel("Preview not available for this format.");
    noPrev->setAlignment(Qt::AlignCenter);
    previewStack->addWidget(noPrev);

    imagePreviewLabel = new QLabel();
    imagePreviewLabel->setAlignment(Qt::AlignCenter);
    imagePreviewLabel->setScaledContents(true);
    previewStack->addWidget(imagePreviewLabel);

    textPreviewArea = new QTextEdit();
    textPreviewArea->setReadOnly(true);
    textPreviewArea->setStyleSheet("font-family: monospace; font-size: 11px; background: transparent; border: none;");
    previewStack->addWidget(textPreviewArea);

    pLayout->addWidget(previewStack);

    fileInfoLabel = new QLabel("");
    fileInfoLabel->setStyleSheet("color: #6b7280; font-size: 12px; border: none;");
    pLayout->addWidget(fileInfoLabel);

    scanSplitter->addWidget(fileGridScroll);
    scanSplitter->addWidget(previewPane);
    connect(closePBtn, &QPushButton::clicked, [this]()
            { previewPane->setVisible(false); });

    stackedWidget->addWidget(scanPage);

    mainLayout->addWidget(stackedWidget);
    mainLayout->addWidget(bottomBar);
    setCentralWidget(mainWidget);

    driveButtonGroup = new QButtonGroup(this);
    driveButtonGroup->setExclusive(true);
    connect(driveButtonGroup, &QButtonGroup::buttonClicked, [this](QAbstractButton *)
            { nextBtn->setEnabled(true); });

    connect(nextBtn, &QPushButton::clicked, this, [this]()
            {
        int currentIndex = stackedWidget->currentIndex();
        if (currentIndex == 0)
        {
            QAbstractButton *selected = driveButtonGroup->checkedButton();
            if (selected)
            {
                selectedSourcePath = selected->property("drivePath").toString();
                selectedSourceSize = static_cast<uint64_t>(selected->property("driveSize").toULongLong());
                
                sidebar->clear();

                vector<Utils::DriveInfo> allDrives = Utils::listDrives();
                QString sourceMountPoint = "";
                for (const auto& drive : allDrives) {
                    if (drive.path == selectedSourcePath.toStdString()) {
                        sourceMountPoint = QString::fromStdString(drive.mountPoint);
                    }
                }

                if (auto filter = qobject_cast<PathFilterProxyModel*>(destFilter)) {
                    filter->setExcludedPath(sourceMountPoint);
                }

                QString firstMountPoint = "";
                for (const auto& drive : allDrives) {
                    if (drive.path == selectedSourcePath.toStdString())
                        continue;

                    if (!drive.mountPoint.empty()) {
                        if (firstMountPoint.isEmpty()) firstMountPoint = QString::fromStdString(drive.mountPoint);
                        QListWidgetItem *item = new QListWidgetItem(QIcon("../assets/drive.svg"), QString::fromStdString(drive.path + " (" + drive.name + ")"));
                        item->setData(Qt::UserRole, QString::fromStdString(drive.mountPoint));
                        sidebar->addItem(item);
                    }
                }

                if (!firstMountPoint.isEmpty()) {
                    QModelIndex sourceIdx = destModel->index(firstMountPoint);
                    QModelIndex proxyIdx = destFilter->mapFromSource(sourceIdx);
                    destTreeView->setRootIndex(proxyIdx);
                    destTreeView->setCurrentIndex(proxyIdx);
                    destSelectedLabel->setText("Selected Destination Base: " + firstMountPoint);
                    nextBtn->setEnabled(true);
                }

                stackedWidget->setCurrentIndex(1);
                nextBtn->setText("Start Scan");
                prevBtn->setVisible(true);
                pathEdit->setVisible(true);
            }
        }
        else if (currentIndex == 1)
        {
            QModelIndex proxyIndex = destTreeView->selectionModel()->currentIndex();
            QModelIndex sourceIndex = destFilter->mapToSource(proxyIndex);
            if (sourceIndex.isValid())
            {
                QString selectedPath = destModel->filePath(sourceIndex);
                QFileInfo fi(selectedPath);
                if (fi.isFile()) selectedPath = fi.absolutePath();

                QString editText = pathEdit->text().trimmed();
                
                if (QDir::isAbsolutePath(editText))
                {
                    finalRecoveryPath = editText;
                }
                else
                {
                    recoveryFolderName = editText.isEmpty() ? "saferestored_data" : editText;
                    finalRecoveryPath = QDir(selectedPath).filePath(recoveryFolderName);
                }
                
                qDebug() << "Selected Base Path:" << selectedPath;
                qDebug() << "Recovery Folder:" << recoveryFolderName;
                qDebug() << "Final Recovery Path:" << finalRecoveryPath;
                QString confirmMsg = QString(
                    "<p>You are about to start the recovery process with the following settings:</p>"
                    "<p style='margin-left:10px;'>"
                    "<b>• Source:</b> <span style='color:#3b82f6;'>%1</span><br>"
                    "<b>• Destination:</b> <span style='color:#3b82f6;'>%2</span><br>"
                    "</p>"
                    "<p>This may take a while depending on the drive size.</p>"
                ).arg(selectedSourcePath, finalRecoveryPath);

                ConfirmDialog diag("Confirm Recovery", confirmMsg, this);
                if (diag.exec() == QDialog::Accepted)
                {
                    startScan();
                    pathEdit->setVisible(false);
                    nextBtn->setText("Recover");
                }
            }
            else
            {
                QMessageBox::warning(this, "Destination Required", "Please select a destination folder where the recovered files will be saved.");
            }
        }
        else if (currentIndex == 2)
        {
            DriveReader reader(selectedSourcePath.toStdString());
            if (!reader.openDrive()) return;

            vector<uint8_t> bootSector;
            if (!reader.readSector(0, bootSector, 512)) return;

            Utils::FileSystemType type = Utils::detectFileSystem(bootSector.data());
            if (type == Utils::FileSystemType::FAT32)
            {
                Fat32Parser parser;
                if (parser.init(bootSector))
                {
                    for (int i = 0; i < fileGridLayout->count(); ++i) {
                        QWidget *card = fileGridLayout->itemAt(i)->widget();
                        if (card) {
                            QCheckBox *cb = card->findChild<QCheckBox *>();
                            if (cb && cb->isChecked()) {
                                int fileIdx = card->property("fileIndex").toInt();
                                if (fileIdx >= 0 && fileIdx < (int)scannedFiles.size()) {
                                    const auto& file = scannedFiles[fileIdx];
                                    parser.recoverFile(reader, file.fullPath, file.startCluster, file.fileSize);
                                }
                            }
                        }
                    }
                }
            }
            ConfirmDialog diag("Recovery Complete", "Selected files have been recovered to the destination folder.", this);
            diag.exec();
        } 
    });

    connect(prevBtn, &QPushButton::clicked, this, [this]()
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
        driveCard->setProperty("driveSize", static_cast<qulonglong>(drive.size));
        driveCard->setProperty("mountPoint", QString::fromStdString(drive.mountPoint));

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
                               "   background-color: #f3f4f6;"
                               "   text-align: center;"
                               "   color: transparent;"
                               "}"
                               "QProgressBar::chunk {"
                               "   background-color: #3b82f6;"
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

QWidget *MainWindow::createFileCard(const QString &fileName, const QString &fileType, const QString &filePath, uint32_t startCluster, uint32_t fileSize)
{
    QWidget *card = new QWidget();
    card->setFixedSize(150, 170);
    card->setObjectName("fileCard");
    card->setStyleSheet(
        "QWidget#fileCard { background-color: white; border: 1px solid #e5e7eb; border-radius: 8px; }"
        "QWidget#fileCard:hover { border: 2px solid #3b82f6; }");

    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(8, 8, 8, 8);
    QHBoxLayout *topRow = new QHBoxLayout();
    QCheckBox *check = new QCheckBox();
    check->setStyleSheet("QCheckBox { spacing: 8px;}"
                         "QCheckBox::indicator {"
                         "width: 18px;"
                         "height: 18px;"
                         "border-radius: 4px;"
                         "border: 2px solid #888;"
                         "transition: all 0.2s;}"

                         "QCheckBox::indicator:checked {"
                         "   background-color: #3b82f6;"
                         "  border: 2px solid #3b82f6;"
                         "image: url(../assets/tick.svg);"
                         "}");
    QToolButton *eyeBtn = new QToolButton();
    eyeBtn->setFixedSize(28, 28);
    eyeBtn->setIcon(QIcon("../assets/eye.svg"));
    eyeBtn->setStyleSheet("border: none; background: #f3f4f6; border-radius: 4px; padding: 3px;");

    topRow->addWidget(check);
    topRow->addStretch();
    topRow->addWidget(eyeBtn);
    layout->addLayout(topRow);

    QLabel *icon = new QLabel();
    icon->setPixmap(QIcon("../assets/file.svg").pixmap(50, 50));
    icon->setAlignment(Qt::AlignCenter);
    layout->addWidget(icon);

    QLabel *name = new QLabel(fileName + "." + fileType.toLower());
    name->setAlignment(Qt::AlignCenter);
    name->setWordWrap(true);
    name->setStyleSheet("font-weight: bold; color: #1f2937; font-size: 14px;");
    layout->addWidget(name);

    connect(eyeBtn, &QToolButton::clicked, this, [this, filePath, fileName, fileType, startCluster, fileSize]()
            { updatePreviewPane(filePath, fileName, fileType, startCluster, fileSize); });

    return card;
}

void MainWindow::updatePreviewPane(const QString &filePath, const QString &fileName, const QString &ext, uint32_t startCluster, uint32_t fileSize)
{
    if (!previewPane->isVisible())
    {
        previewPane->setVisible(true);
    }

    previewFileNameLabel->setText(fileName + "." + ext.toLower());
    QString lowerExt = ext.toLower();

    vector<uint8_t> fileData;
    DriveReader reader(selectedSourcePath.toStdString());
    if (reader.openDrive())
    {
        Fat32Parser parser;
        vector<uint8_t> bootSector;
        if (reader.readSector(0, bootSector, 512))
        {
            if (parser.init(bootSector))
            {
                fileData = parser.readFileData(reader, startCluster, fileSize);
            }
        }
    }

    if (fileData.empty())
    {
        previewStack->setCurrentIndex(2);
        textPreviewArea->setPlainText("Could not read file data for preview. Ensure the drive is still connected.");
        return;
    }

    bool isImage = false;
    QString detectedType = "Unknown";

    if (fileData.size() >= 4)
    {
        if (fileData[0] == 0xFF && fileData[1] == 0xD8 && fileData[2] == 0xFF)
        {
            isImage = true;
            detectedType = "JPEG Image";
        }
        else if (fileData[0] == 0x89 && fileData[1] == 0x50 && fileData[2] == 0x4E && fileData[3] == 0x47)
        {
            isImage = true;
            detectedType = "PNG Image";
        }
        else if (fileData[0] == 0x42 && fileData[1] == 0x4D)
        {
            isImage = true;
            detectedType = "BMP Image";
        }
        else if (fileData[0] == 0x47 && fileData[1] == 0x49 && fileData[2] == 0x46 && fileData[3] == 0x38)
        {
            isImage = true;
            detectedType = "GIF Image";
        }
        else if (fileData[0] == 0x25 && fileData[1] == 0x50 && fileData[2] == 0x44 && fileData[3] == 0x46)
        {
            detectedType = "PDF Document";
        }
        else if (fileData[0] == 0x50 && fileData[1] == 0x4B && fileData[2] == 0x03 && fileData[3] == 0x04)
        {
            detectedType = "ZIP / Office Archive";
        }
        else if (fileData[0] == 0x49 && fileData[1] == 0x44 && fileData[2] == 0x33)
        {
            detectedType = "MP3 Audio";
        }
        else if (fileData.size() >= 8 && fileData[4] == 0x66 && fileData[5] == 0x74 && fileData[6] == 0x79 && fileData[7] == 0x70)
        {
            detectedType = "MP4 / MOV Video";
        }
        else if (fileData[0] == 0x4D && fileData[1] == 0x5A)
        {
            detectedType = "Windows Executable (EXE/DLL)";
        }
    }

    if (!isImage && detectedType == "Unknown")
    {
        if (lowerExt == "jpg" || lowerExt == "jpeg" || lowerExt == "png" || lowerExt == "bmp" || lowerExt == "gif")
        {
            isImage = true;
            detectedType = "Image (via Extension)";
        }
    }

    if (isImage)
    {
        previewStack->setCurrentIndex(1);
        QPixmap pixmap;
        if (pixmap.loadFromData(fileData.data(), fileData.size()))
        {
            imagePreviewLabel->setPixmap(pixmap.scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else
        {
            previewStack->setCurrentIndex(2);
            textPreviewArea->setPlainText("Detected as " + detectedType + ", but data appears corrupted.");
        }
    }
    else
    {
        previewStack->setCurrentIndex(2);
        size_t previewSize = min((size_t)4096, fileData.size());
        bool isText = true;
        for (size_t i = 0; i < min(previewSize, (size_t)100); ++i)
        {
            if (fileData[i] < 32 && fileData[i] != '\n' && fileData[i] != '\r' && fileData[i] != '\t')
            {
                isText = false;
                break;
            }
        }

        if (isText || lowerExt == "txt" || lowerExt == "log" || lowerExt == "ini")
        {
            QString textPreview = QString::fromUtf8((char *)fileData.data(), previewSize);
            textPreviewArea->setPlainText("Detected Format: Text / Log File\n\n" + textPreview);
        }
        else
        {
            QByteArray data = QByteArray::fromRawData((char *)fileData.data(), previewSize);
            textPreviewArea->setPlainText("Detected Format: " + detectedType + "\n\nBinary Data (Hex View):\n\n" + data.toHex(' ').toUpper());
        }
    }

    fileInfoLabel->setText(QString("Size: %1").arg(QString::fromStdString(Utils::formatSize(fileSize))));
    relayoutFiles();
}

void MainWindow::startScan()
{
    DriveReader reader(selectedSourcePath.toStdString());
    if (!reader.openDrive())
    {
        QMessageBox::critical(this, "Error", "Could not open drive. Please ensure you are running with root privileges (sudo).");
        return;
    }

    vector<uint8_t> bootSector;
    if (!reader.readSector(0, bootSector, 512))
    {
        QMessageBox::critical(this, "Error", "Failed to read the boot sector. The drive might be disconnected.");
        return;
    }

    Utils::FileSystemType type = Utils::detectFileSystem(bootSector.data());
    QString fsName = (type == Utils::FileSystemType::FAT32) ? "FAT32" : "Unknown";

    if (type == Utils::FileSystemType::UNKNOWN)
    {
        QMessageBox::warning(this, "File System Not Supported", "The selected partition uses an unsupported file system. SafeRestore currently only supports FAT32.");
        return;
    }

    bool allZeros = true;
    for (int i = 0; i < 16; ++i)
        if (bootSector[i] != 0)
            allZeros = false;
    if (allZeros)
    {
        QMessageBox::critical(this, "Data Error", "The boot sector was read as all zeros. Ensure root privileges.");
        return;
    }

    BaseParser *recoveryEngine = nullptr;
    if (type == Utils::FileSystemType::FAT32)
        recoveryEngine = new Fat32Parser();

    if (recoveryEngine)
    {
        if (recoveryEngine->init(bootSector))
        {
            QLayoutItem *item;
            while ((item = fileGridLayout->takeAt(0)) != nullptr)
            {
                if (item->widget())
                    item->widget()->deleteLater();
                delete item;
            }
            fileCardWidgets.clear();

            QLabel *loadingLabel = new QLabel("Scanning drive... Please wait.");
            loadingLabel->setAlignment(Qt::AlignCenter);
            loadingLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #3b82f6; margin-top: 50px;");
            fileGridLayout->addWidget(loadingLabel);
            QApplication::processEvents();

            if (!QDir().exists(finalRecoveryPath))
            {
                QDir().mkpath(finalRecoveryPath);
            }
            vector<RecoveredFileInfo> foundFiles = recoveryEngine->scan(reader, finalRecoveryPath.toStdString(), selectedSourceSize);

            while ((item = fileGridLayout->takeAt(0)) != nullptr)
            {
                if (item->widget())
                    item->widget()->deleteLater();
                delete item;
            }

            scannedFiles = foundFiles;
            if (foundFiles.empty())
            {
                QLabel *noFilesLabel = new QLabel("No deleted files found on this " + fsName + " drive.");
                noFilesLabel->setStyleSheet("font-size: 16px; color: #6b7280;");
                fileGridLayout->addWidget(noFilesLabel);
            }
            else
            {
                for (size_t i = 0; i < foundFiles.size(); ++i)
                {
                    const auto &file = foundFiles[i];
                    QWidget *card = createFileCard(QString::fromStdString(file.fileName), QString::fromStdString(file.extension), QString::fromStdString(file.fullPath), file.startCluster, file.fileSize);
                    card->setProperty("fileIndex", (int)i);
                    fileCardWidgets.push_back(card);
                    fileGridLayout->addWidget(card);
                }
                relayoutFiles();
            }
        }
        else
        {
            QMessageBox::warning(this, "Parser Error", "Failed to initialize parser.");
        }
        delete recoveryEngine;
    }

    if (stackedWidget->currentIndex() != 2)
    {
        stackedWidget->setCurrentIndex(2);
    }
}

void MainWindow::relayoutFiles()
{
    if (fileGridLayout)
    {
        fileGridLayout->invalidate();
        fileGridLayout->activate();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    relayoutFiles();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        QWidget *clickedWidget = childAt(event->pos());
        if (clickedWidget && (clickedWidget == topBar || topBar->isAncestorOf(clickedWidget)))
        {
            isDragging = true;
            dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
        }
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (isDragging && (event->buttons() & Qt::LeftButton))
    {
        move(event->globalPosition().toPoint() - dragPosition);
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    isDragging = false;
}

MainWindow::~MainWindow()
{
}
#include "moc_MainWindow.cpp"