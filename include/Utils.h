#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <string>
#include <vector>

using namespace std;

namespace Utils
{

    enum class FileSystemType {
        FAT32,
        NTFS,
        EXFAT,
        UNKNOWN
    };

    struct DriveInfo
    {
        string path;
        string name;
        uint64_t size;
    };

    string formatSize(uint64_t bytes);

    void hexDump(const uint8_t *data, size_t size);

    vector<DriveInfo> listDrives();

    FileSystemType detectFileSystem(const uint8_t* sector0);

    string fsTypeToString(FileSystemType type);

    FileSystemType getDriveType(const std::string& path);
}

#endif