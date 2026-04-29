#ifndef BASEPARSER_H
#define BASEPARSER_H

#include <string>
#include <vector>
#include "DriveReader.h"

using namespace std;

class BaseParser
{
public:
    virtual ~BaseParser() {}
    virtual bool init(const vector<uint8_t> &bootSector) = 0;
    virtual void scan(DriveReader &reader, const string &dstPath, uint64_t driveSize) = 0;
    virtual string getFsName() const = 0;
};

#endif