#ifndef DRIVEREADER_H
#define DRIVEREADER_H

#include <string>
#include <vector>
#include <cstdint>

using namespace std;

class DriveReader
{
    private:
    int fileDescriptor;
    string devicePath;

    public:
    DriveReader(const string &path);
    ~DriveReader();

    bool openDrive();
    bool readSector(uint64_t offset, vector<uint8_t> &buffer, size_t bytesToRead = 512);
    void closeDrive();
};

#endif