#ifndef FAT32PARSER_H
#define FAT32PARSER_H

#include "BaseParser.h"

using namespace std;

class Fat32Parser : public BaseParser
{
private:
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t numFats;
    uint32_t sectorsPerFat;
    uint32_t rootCluster;
    uint64_t fatStartByte;
    uint64_t dataStartByte;
    uint64_t clusterToByte(uint32_t cluster);

public:
    Fat32Parser();
    bool init(const vector<uint8_t> &bootSector) override;
    void scan(DriveReader &reader, const string &destPath, uint64_t driveSize) override;
    string getFsName() const override { return "FAT32"; }
    void recoverFile(DriveReader &reader, const string& outPath, uint32_t startCluster, uint32_t fileSize);
};

#endif