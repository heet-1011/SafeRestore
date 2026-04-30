#include "../include/MainWindow.h"
#include "../include/DriveReader.h"
#include "../include/Utils.h"
#include "../include/BaseParser.h"
#include "../include/Fat32Parser.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <filesystem>
#include <string>
#include <QApplication>
#include <QSplashScreen>
#include <QPainter>
#include <QTimer>
#include <QThread>
#include <QSvgRenderer>
#include <QPainterPath>
#include <QMouseEvent>

using namespace std;

namespace fs = filesystem;

class LockedSplashScreen : public QSplashScreen
{
public:
    using QSplashScreen::QSplashScreen;

protected:
    void mousePressEvent(QMouseEvent *event) override
    {
        event->accept();
    }
};

int runCommandLineMode()
{
    cout << "========================================\n";
    cout << "      SafeRestore: Pro Data Recovery    \n";
    cout << "----------------------------------------\n";
    cout << "            Developed by HD             \n";
    cout << "========================================\n\n";

    vector<Utils::DriveInfo> drives = Utils::listDrives();
    if (drives.empty())
    {
        cerr << "[-] No physical drives detected. Run with sudo!\n";
        return 1;
    }

    cout << "[+] Available Physical Drives:\n";
    for (size_t i = 0; i < drives.size(); ++i)
    {
        cout << "  " << i + 1 << ") " << drives[i].name << " (" << Utils::formatSize(drives[i].size) << ") at " << drives[i].path << "\n";
    }

    int choice;
    cout << "\nSelect drive to scan (1-" << drives.size() << "): ";
    cin >> choice;

    if (choice < 1 || choice > (int)drives.size())
    {
        cerr << "[-] Invalid selection.\n";
        return 1;
    }
    Utils::DriveInfo selectedDrive = drives[choice - 1];

    Utils::FileSystemType type = Utils::getDriveType(selectedDrive.path);

    cout << "[+] Selected Drive: " << selectedDrive.name << "\n";
    cout << "[+] Detected File System: " << Utils::fsTypeToString(type) << "\n";

    string destPath;
    cout << "[?] Where should recovered files be saved? (e.g., /home/user/Recovered): ";
    cin >> destPath;

    try
    {
        if (!fs::exists(destPath))
        {
            fs::create_directories(destPath);
            cout << "[+] Created directory: " << destPath << "\n";
        }
    }
    catch (const exception &e)
    {
        cerr << "[-] Error creating directory: " << e.what() << "\n";
        return 1;
    }

    DriveReader reader(selectedDrive.path);
    if (!reader.openDrive())
    {
        cerr << "[-] Failed to open " << selectedDrive.path << "\n";
        return 1;
    }

    cout << "\n[!] Initialization Complete.\n";
    cout << "    Source: " << selectedDrive.path << "\n";
    cout << "    Output: " << fs::absolute(destPath) << "\n";
    cout << "----------------------------------------\n";

    vector<uint8_t> buffer;
    if (reader.readSector(0, buffer, 512))
    {
        cout << "[+] Sector 0 Header Check:\n";
        Utils::hexDump(buffer.data(), 32);
    }

    cout << "\n[OK] Ready for deep scan. Proceed? (y/n): ";
    char confirm;
    cin >> confirm;

    if (confirm == 'y' || confirm == 'Y')
    {
        cout << "[*] Starting scan logic...\n";
        BaseParser *recoveryEngine = nullptr;

        if (type == Utils::FileSystemType::FAT32)
        {
            recoveryEngine = new Fat32Parser();
        }
        else
        {
            cout << "[-] Automatic undelete is only supported for FAT32 right now.\n";
        }

        if (recoveryEngine)
        {
            vector<uint8_t> bootSector;
            if (reader.readSector(0, bootSector, 512))
            {

                if (recoveryEngine->init(bootSector))
                {
                    recoveryEngine->scan(reader, destPath, selectedDrive.size);
                }
            }
            delete recoveryEngine;
        }
    }

    reader.closeDrive();
    return 0;
}

int main(int argc, char *argv[])
{

    if (argc > 1)
    {
        return runCommandLineMode();
    }

    QApplication app(argc, argv);
    int sWidth = 800;
    int sHeight = 500;
    QPixmap pixmap(sWidth, sHeight);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath path;
    path.addRoundedRect(0, 0, sWidth, sHeight, 20, 20);
    painter.setClipPath(path);
    QPixmap bgImage("../assets/bg.jpg");
    if (bgImage.isNull())
    {
        painter.fillRect(0, 0, sWidth, sHeight, QColor("#001833"));
    }
    else
    {
        QPixmap scaledBg = bgImage.scaledToHeight(sHeight, Qt::SmoothTransformation);
        int xPos = -200;
        painter.drawPixmap(xPos, 0, scaledBg);
    }

    int panelWidth = sWidth / 2.5;
    int panelStartX = sWidth - panelWidth;
    painter.setBrush(QColor("#001833"));
    painter.setPen(Qt::NoPen);
    painter.drawRect(panelStartX, 0, panelWidth, sHeight);

    int logoSize = 80;
    int logoY = 130;
    int containerSize = 80;
    int containerX = panelStartX + (panelWidth / 2) - (containerSize / 2);
    int containerY = 130;

    painter.setBrush(QColor("#002a52"));
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(containerX, containerY, containerSize, containerSize, 10, 10);

    QSvgRenderer svgLogoRenderer(QString("../assets/safe-restore.svg"));
    if (svgLogoRenderer.isValid())
    {
        int padding = 15;
        int svgSize = containerSize - (padding * 2);
        QRectF svgRect(containerX + padding, containerY + padding, svgSize, svgSize);
        svgLogoRenderer.render(&painter, svgRect);
    }
    else
    {
        painter.setBrush(QColor("#22BFEF"));
        painter.drawEllipse(containerX + 20, containerY + 20, containerSize - 40, containerSize - 40);
    }

    painter.setPen(QColor("#FFFFFF"));
    painter.setFont(QFont("Inter", 22, QFont::Bold));

    int textY = logoY + logoSize + 20;
    painter.drawText(panelStartX, textY, panelWidth, 40, Qt::AlignCenter, "SafeRestore");
    painter.setPen(QColor("#FFFFFF"));
    painter.setFont(QFont("Inter", 9, QFont::Medium));
    int devTextY = sHeight - 40;
    painter.drawText(panelStartX, devTextY, panelWidth, 20, Qt::AlignCenter, "Developed by HD");
    painter.end();

    LockedSplashScreen splash(pixmap);
    splash.setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    splash.setAttribute(Qt::WA_TranslucentBackground);
    splash.show();

    MainWindow win;
    QTimer::singleShot(3000, [&]()
                       {
        win.show();
        splash.finish(&win); });
    return app.exec();
}