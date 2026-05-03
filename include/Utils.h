#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <string>
#include <vector>

using namespace std;

namespace Utils
{

    enum class FileSystemType
    {
        FAT32,
        NTFS,
        EXFAT,
        UNKNOWN
    };

    struct DriveInfo
    {
        string path;
        string name;
        string mountPoint;
        uint64_t size;
        bool isExternal;
        uint64_t usedSize;
    };

    string formatSize(uint64_t bytes);

    void hexDump(const uint8_t *data, size_t size);

    vector<DriveInfo> listDrives();

    FileSystemType detectFileSystem(const uint8_t *sector0);

    string fsTypeToString(FileSystemType type);

    FileSystemType getDriveType(const string &path);

    uint64_t getUsedSpaceForDrive(const string &driveName);

    string getPartitionType(const string &devPath);
    string getMountPoint(const string &driveName);

}

#endif