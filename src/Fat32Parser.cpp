#include "../include/Fat32Parser.h"
#include <iostream>
#include <cstring>
#include <fstream>
#include <algorithm>

using namespace std;

Fat32Parser::Fat32Parser() : bytesPerSector(0), sectorsPerCluster(0),
                             reservedSectors(0), numFats(0),
                             sectorsPerFat(0), rootCluster(0) {}

bool Fat32Parser::init(const vector<uint8_t> &bootSector)
{
    if (bootSector.size() < 90)
        return false;

    bytesPerSector = bootSector[11] | (bootSector[12] << 8);
    sectorsPerCluster = bootSector[13];
    reservedSectors = bootSector[14] | (bootSector[15] << 8);
    numFats = bootSector[16];

    memcpy(&sectorsPerFat, &bootSector[36], 4);
    memcpy(&rootCluster, &bootSector[44], 4);

    fatStartByte = (uint64_t)reservedSectors * bytesPerSector;
    uint64_t fatRegionSize = (uint64_t)numFats * sectorsPerFat * bytesPerSector;
    dataStartByte = fatStartByte + fatRegionSize;

    cout << "[Parser] Bytes/Sector: " << bytesPerSector << "\n";
    cout << "[Parser] Sectors/Cluster: " << (int)sectorsPerCluster << "\n";
    cout << "[Parser] Data Region starts at byte: " << dataStartByte << "\n";

    return true;
}

uint64_t Fat32Parser::clusterToByte(uint32_t cluster)
{
    return dataStartByte + (uint64_t)(cluster - 2) * sectorsPerCluster * bytesPerSector;
}

void Fat32Parser::scan(DriveReader &reader, const string &destPath, uint64_t driveSize)
{
    cout << "[*] Searching for deleted entries in Root Directory...\n";

    uint64_t rootByteOffset = clusterToByte(rootCluster);
    vector<uint8_t> buffer;
    uint32_t clusterSize = bytesPerSector * sectorsPerCluster;

    if (reader.readSector(rootByteOffset, buffer, bytesPerSector * sectorsPerCluster))
    {
        int foundCount = 0;
        for (size_t i = 0; i < buffer.size(); i += 32)
        {
            if (buffer[i] == 0xE5)
            {

                uint8_t attribute = buffer[i + 11];
                if (attribute == 0x0F)
                    continue;

                string fileName = "";
                for (int j = 1; j < 11; ++j)
                {
                    if (buffer[i + j] != ' ')
                        fileName += (char)buffer[i + j];
                }

                uint16_t high = buffer[i + 20] | (buffer[i + 21] << 8);
                uint16_t low = buffer[i + 26] | (buffer[i + 27] << 8);
                uint32_t startCluster = low | (high << 16);

                uint32_t fileSize;
                memcpy(&fileSize, &buffer[i + 28], 4);

                uint64_t physicalOffset = clusterToByte(startCluster);

                if (startCluster < 2 || physicalOffset >= driveSize || (physicalOffset + fileSize) > driveSize)
                {
                    continue;
                }

                foundCount++;
                cout << "\n[!] DELETED FILE FOUND #" << foundCount << "\n";
                cout << "    Offset: " << i << "\n";
                cout << "    Name: _" << fileName << "\n";
                cout << "    Size: " << fileSize << " bytes\n";
                cout << "    Start Cluster: " << startCluster << "\n";

                string fullOutPath = destPath + "/" + fileName;

                cout << "    Attempting recovery...\n";
                recoverFile(reader, fullOutPath, startCluster, fileSize);
            }
        }
        if (foundCount == 0)
        {
            cout << "[-] No deleted files found in the root directory.\n";
        }
    }
}

void Fat32Parser::recoverFile(DriveReader &reader, const string &outPath, uint32_t startCluster, uint32_t fileSize)
{
    if (fileSize == 0)
        return;

    ofstream outFile(outPath, ios::binary);
    if (!outFile.is_open())
    {
        cerr << "    [ERROR] Could not create output file: " << outPath << "\n";
        return;
    }

    uint32_t bytesRemaining = fileSize;
    uint32_t currentCluster = startCluster;
    uint32_t clusterSize = (uint32_t)bytesPerSector * sectorsPerCluster;

    cout << "    [PROCESS] Starting recovery from Cluster " << startCluster << " (" << fileSize << " bytes)\n";

    while (bytesRemaining > 0)
    {
        uint64_t offset = clusterToByte(currentCluster);

        uint32_t toRead = min(bytesRemaining, clusterSize);

        vector<uint8_t> clusterBuffer;

        if (reader.readSector(offset, clusterBuffer, toRead))
        {
            outFile.write((char *)clusterBuffer.data(), toRead);

            bytesRemaining -= toRead;

            currentCluster++;
        }
        else
        {
            cerr << "    [ERROR] Failed to read hardware at offset " << offset << "\n";
            break;
        }

        if (currentCluster == 0)
            break;
    }

    outFile.close();

    if (bytesRemaining == 0)
    {
        cout << "    [SUCCESS] File fully recovered to: " << outPath << "\n";
    }
    else
    {
        cout << "    [WARNING] Partial recovery. " << bytesRemaining << " bytes missing.\n";
    }
}