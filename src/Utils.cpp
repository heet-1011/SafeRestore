#include "../include/Utils.h"
#include "../include/DriveReader.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <cstring>
#include <sys/statvfs.h>
#include <mntent.h>
#include <blkid/blkid.h>
#include <algorithm>

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

        for (const auto &entry : fs::directory_iterator("/sys/class/block/"))
        {
            string name = entry.path().filename().string();

            if (name.find("loop") == 0 || name.find("ram") == 0)
                continue;

            bool isPartition = fs::exists(entry.path() / "partition");
            if (!isPartition)
            {
                bool hasPartitions = false;
                string blockPath = "/sys/block/" + name;
                if (fs::exists(blockPath))
                {
                    for (const auto &sub : fs::directory_iterator(blockPath))
                    {
                        string subName = sub.path().filename().string();
                        if (subName.find(name) == 0 && isdigit(subName.back()))
                        {
                            hasPartitions = true;
                            break;
                        }
                    }
                }
                if (hasPartitions)
                {
                    continue;
                }
            }

            DriveInfo info;
            info.name = name;
            info.path = "/dev/" + name;

            ifstream sizeFile(entry.path() / "size");
            uint64_t sectors;
            if (sizeFile >> sectors)
            {
                info.size = sectors * 512;

                string partType = getPartitionType(info.path);
                if (partType == "c12a7328-f81f-11d2-ba4b-00a0c93ec93b" || // EFI
                    partType == "e3c9e316-0b5c-4db8-817d-f92df00215ae" || // MS Reserved
                    partType == "de94bba4-06d1-4d40-a16a-bfd50179d6ac" || // Win Recovery
                    partType == "0657fd6d-a4ab-43c4-84e5-0933c84b4f4f" || // Linux Swap
                    partType == "ef" || partType == "82")                 // MBR Boot/Swap
                {
                    continue;
                }

                string parentName = name;
                while (!parentName.empty() && isdigit(parentName.back()))
                {
                    parentName.pop_back();
                }
                if (!parentName.empty() && parentName.back() == 'p' &&
                    (name.find("nvme") == 0 || name.find("mmcblk") == 0))
                {
                    parentName.pop_back();
                }

                info.isExternal = false;
                ifstream removableFile("/sys/block/" + parentName + "/removable");
                int removableFlag;
                if (removableFile >> removableFlag)
                {
                    if (removableFlag == 1)
                        info.isExternal = true;
                }
                if (name.find("nvme") == 0)
                {
                    info.isExternal = false;
                }

            info.usedSize = getUsedSpaceForDrive(name);
            info.mountPoint = getMountPoint(name);

            if (info.size > 0)
            {
                drives.push_back(info);
            }
        }
    }
    return drives;
}

    uint64_t getUsedSpaceForDrive(const string &driveName)
    {
        uint64_t totalUsed = 0;

        FILE *mounts = setmntent("/proc/mounts", "r");
        if (!mounts)
            return 0;

        struct mntent *ent;
        while ((ent = getmntent(mounts)) != nullptr)
        {
            string fsname = ent->mnt_fsname;

            if (fsname == "/dev/" + driveName || fsname.find("/dev/" + driveName) == 0)
            {
                struct statvfs stats;
                if (statvfs(ent->mnt_dir, &stats) == 0)
                {
                    uint64_t totalSpace = (uint64_t)stats.f_blocks * stats.f_frsize;
                    uint64_t freeSpace = (uint64_t)stats.f_bfree * stats.f_frsize;
                    totalUsed += (totalSpace - freeSpace);
                }
            }
        }
        endmntent(mounts);

        return totalUsed;
    }

    string getMountPoint(const string &driveName)
    {
        FILE *mounts = setmntent("/proc/mounts", "r");
        if (!mounts)
            return "";

        struct mntent *ent;
        string mountPoint = "";
        while ((ent = getmntent(mounts)) != nullptr)
        {
            string fsname = ent->mnt_fsname;
            if (fsname == "/dev/" + driveName || fsname.find("/dev/" + driveName) == 0)
            {
                mountPoint = ent->mnt_dir;
                break;
            }
        }
        endmntent(mounts);
        return mountPoint;
    }

    string getPartitionType(const string &devPath)
    {
        string result = "";

        blkid_probe pr = blkid_new_probe_from_filename(devPath.c_str());
        if (!pr)
            return result;
        blkid_probe_enable_partitions(pr, 1);
        blkid_probe_set_partitions_flags(pr, BLKID_PARTS_ENTRY_DETAILS);

        if (blkid_do_safeprobe(pr) == 0)
        {
            const char *data;
            if (blkid_probe_lookup_value(pr, "PART_ENTRY_TYPE", &data, NULL) == 0)
            {
                result = data;
            }
        }

        blkid_free_probe(pr);
        transform(result.begin(), result.end(), result.begin(), ::tolower);
        return result;
    }

    FileSystemType detectFileSystem(const uint8_t *sector0)
    {
        if (memcmp(&sector0[3], "NTFS    ", 8) == 0)
        {
            return FileSystemType::NTFS;
        }

        if (memcmp(&sector0[3], "EXFAT   ", 8) == 0)
        {
            return FileSystemType::EXFAT;
        }

        if (memcmp(&sector0[82], "FAT32   ", 8) == 0)
        {
            return FileSystemType::FAT32;
        }

        return FileSystemType::UNKNOWN;
    }

    string fsTypeToString(FileSystemType type)
    {
        switch (type)
        {
        case FileSystemType::FAT32:
            return "FAT32";
        case FileSystemType::NTFS:
            return "NTFS";
        case FileSystemType::EXFAT:
            return "exFAT";
        default:
            return "UNKNOWN/RAW";
        }
    }

    FileSystemType getDriveType(const string &path)
    {
        DriveReader tempReader(path);

        if (!tempReader.openDrive())
        {
            return FileSystemType::UNKNOWN;
        }

        vector<uint8_t> buffer;
        if (tempReader.readSector(0, buffer, 512))
        {
            return detectFileSystem(buffer.data());
        }

        return FileSystemType::UNKNOWN;
    }
}