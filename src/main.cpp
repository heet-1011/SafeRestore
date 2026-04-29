#include <iostream>
#include <iomanip>
#include <vector>
#include <filesystem>
#include "../include/DriveReader.h"
#include "../include/Utils.h"
#include "../include/BaseParser.h"
#include "../include/Fat32Parser.h"

using namespace std;

namespace fs = filesystem;

int main()
{
    cout << "========================================\n";
    cout << "      SafeRestore: Pro Data Recovery    \n";
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