#include "../include/DriveReader.h"
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

DriveReader::DriveReader(const string &path)
{
    devicePath = path;
    fileDescriptor = -1;
}

DriveReader::~DriveReader()
{
    closeDrive();
}

bool DriveReader::openDrive()
{
    fileDescriptor = open(devicePath.c_str(), O_RDONLY);

    if (fileDescriptor < 0)
    {
        cerr << "Error: Could not open " << devicePath << ". (Have you used sudo?)\n";
        return false;
    }

    return true;
}

bool DriveReader::readSector(uint64_t offset, vector<uint8_t> &buffer, size_t bytesToRead)
{
    if (fileDescriptor < 0)
    {
        cerr << "Error: Drive is not open.\n";
        return false;
    }

    buffer.resize(bytesToRead);

    ssize_t bytesRead = pread(fileDescriptor, buffer.data(), bytesToRead, offset);

    if (bytesRead != bytesToRead)
    {
        cerr << "Error: Could not read the requested bytes at offset " << offset << "\n";
        return false;
    }

    return true;
}

void DriveReader::closeDrive()
{
    if (fileDescriptor >= 0)
    {
        close(fileDescriptor);
        fileDescriptor = -1;
    }
}