#include "../include/Utils.h"
#include "../include/DriveReader.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <cstring>

using namespace std;

namespace fs = filesystem;

namespace Utils
{
    string formatSize(uint64_t bytes)
    {
        const char *suffixes[] = {"B", "KB", "MB", "GB", "TB"};
        int s = 0;
        double dblBytes = bytes;
        while (dblBytes >= 1024 && s < 4)
        {
            dblBytes /= 1024;
            s++;
        }
        char result[50];
        sprintf(result, "%.2f %s", dblBytes, suffixes[s]);
        return string(result);
    }

    void hexDump(const uint8_t *data, size_t size)
    {
        for (size_t i = 0; i < size; ++i)
        {
            cout << hex << uppercase << setw(2) << setfill('0') << (int)data[i] << " ";
            if ((i + 1) % 16 == 0)
                cout << "\n";
        }
        cout << dec << "\n";
    }

    vector<DriveInfo> listDrives()
    {
        vector<DriveInfo> drives;

        for (const auto &entry : fs::directory_iterator("/sys/block/"))
        {
            string name = entry.path().filename().string();

            if (name.find("loop") == 0 || name.find("ram") == 0)
                continue;

            DriveInfo info;
            info.name = name;
            info.path = "/dev/" + name;

            ifstream sizeFile("/sys/block/" + name + "/size");
            uint64_t sectors;
            if (sizeFile >> sectors)
            {
                info.size = sectors * 512;
                drives.push_back(info);
            }
        }
        return drives;
    }

    FileSystemType detectFileSystem(const uint8_t* sector0) {
        // 1. Check for NTFS: Name is at byte 3
        if (memcmp(&sector0[3], "NTFS    ", 8) == 0) {
            return FileSystemType::NTFS;
        }

        // 2. Check for exFAT: Name is at byte 3
        if (memcmp(&sector0[3], "EXFAT   ", 8) == 0) {
            return FileSystemType::EXFAT;
        }

        // 3. Check for FAT32: Name is at byte 82
        if (memcmp(&sector0[82], "FAT32   ", 8) == 0) {
            return FileSystemType::FAT32;
        }

        return FileSystemType::UNKNOWN;
    }

    string fsTypeToString(FileSystemType type) {
        switch (type) {
            case FileSystemType::FAT32: return "FAT32";
            case FileSystemType::NTFS:  return "NTFS";
            case FileSystemType::EXFAT: return "exFAT";
            default:                    return "UNKNOWN/RAW";
        }
    }

    FileSystemType getDriveType(const string& path) {
        DriveReader tempReader(path);
        
        if (!tempReader.openDrive()) {
            return FileSystemType::UNKNOWN;
        }

        vector<uint8_t> buffer;
        if (tempReader.readSector(0, buffer, 512)) {
            return detectFileSystem(buffer.data());
        }

        return FileSystemType::UNKNOWN;
    }
}