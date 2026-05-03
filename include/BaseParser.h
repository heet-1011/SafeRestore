#ifndef BASEPARSER_H
#define BASEPARSER_H

#include "DriveReader.h"
#include <string>
#include <vector>

using namespace std;

struct RecoveredFileInfo {
    string fullOutPath;
    string fileName;
    string extension;
    uint32_t startCluster;
    uint32_t fileSize;
    string fullPath;
};

class BaseParser
{
public:
    virtual ~BaseParser() {}
    virtual bool init(const vector<uint8_t> &bootSector) = 0;
    virtual vector<RecoveredFileInfo> scan(DriveReader &reader, const string &dstPath, uint64_t driveSize) = 0;
    virtual vector<uint8_t> readFileData(DriveReader &reader, uint32_t startCluster, uint32_t fileSize) = 0;
    virtual string getFsName() const = 0;
};

#endif