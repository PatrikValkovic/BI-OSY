#ifndef __PROGTEST__
#include "definitions.h"
#endif

namespace Valkovic
{
    class raidInstance
    {
    private:
        int (* _read )(int, int, void*, int);
        int (* _write )(int, int, const void*, int);
    public:

        int devices;
        int sectors;
        int status = RAID_STOPPED;
        int countOfBrokenDisks;
        int brokenDisks[2];
        unsigned int timestamp;

        void getData(TBlkDev* dev)
        {
            _read = dev->m_Read;
            _write = dev->m_Write;
            devices = dev->m_Devices;
            sectors = dev->m_Sectors;
            status = RAID_OK;
            countOfBrokenDisks = 0;
            brokenDisks[0] = -1;
            brokenDisks[1] = -1;
            timestamp = 1;
        }

        int read(int diskNumber, int sectorNumber, void* data, int sizeInSectors)
        {
            int readed = this->_read(diskNumber, sectorNumber, data, sizeInSectors);
            if (readed != sizeInSectors)
            {
                brokenDisks[countOfBrokenDisks++] = diskNumber;
                if (countOfBrokenDisks <= 2)
                    status = RAID_DEGRADED;
                else
                    status = RAID_FAILED;
            };
            return readed;
        }

        int write(int diskNumber, int sectorNumber, const void* data, int sizeInSectors)
        {
            int wrote = this->_write(diskNumber, sectorNumber, data, sizeInSectors);
            if (wrote != sizeInSectors)
            {
                brokenDisks[countOfBrokenDisks++] = diskNumber;
                if (countOfBrokenDisks <= 2)
                    status = RAID_DEGRADED;
                else
                    status = RAID_FAILED;
            };
            return wrote;
        }

        inline bool isNotBroken(int disk) const
        {
            return countOfBrokenDisks == 0 ||
                   (countOfBrokenDisks == 1 && brokenDisks[0] != disk) ||
                   (countOfBrokenDisks == 2 && brokenDisks[0] != disk && brokenDisks[1] != disk);
        }

        inline bool isBroken(int disk) const
        {
            return !isNotBroken(disk);
        }

        void position(int sector, int& column, int& line) const
        {
            line = sector / (devices - 2);
            int XORcolumn = line % devices;
            int REEDcolumn = (XORcolumn + 1) % devices;

            int sectorAtBeginOfLine = line * (devices - 2);
            int countOfSteps = sector - sectorAtBeginOfLine;
            for (column = 0; countOfSteps > 0 || XORcolumn==column || REEDcolumn==column; column++)
            {
                if (column == XORcolumn || column == REEDcolumn)
                    continue;
                else
                    countOfSteps--;
            }
        }
    };

    static raidInstance raid;
}

int RaidCreate(TBlkDev* dev)
{
    using namespace Valkovic;

    if (dev == nullptr)
        return 0;

    raid.getData(dev);
    raid.status = RAID_STOPPED;

    char servisInformations[SECTOR_SIZE];
    memcpy(servisInformations, &raid.timestamp, sizeof(unsigned int));

    int countOfBroken = 0;
    for (int i = 0; i < raid.devices && countOfBroken <= 2; i++)
        if (raid.write(i, raid.sectors - 1, servisInformations, 1) != 1)
        {
            countOfBroken++;
            raid.brokenDisks[raid.countOfBrokenDisks++] = i;
        }

    if (countOfBroken > 2)
    {
        return 0;
    }
    return 1;
}

void RaidStart(TBlkDev* dev)
{
    using namespace Valkovic;

    if (dev == nullptr)
        return;

    raid.getData(dev);

    unsigned int timestamps[MAX_RAID_DEVICES];
    memset(timestamps, 0, sizeof(unsigned int) * MAX_RAID_DEVICES);
    int timestampsPosition[MAX_RAID_DEVICES];
    char data[SECTOR_SIZE];

    for (int i = 0; i < raid.devices && raid.countOfBrokenDisks <= 2; i++)
    {
        if (raid.read(i, raid.sectors - 1, data, 1) != 1)
        {
            raid.brokenDisks[raid.countOfBrokenDisks++] = i;
            continue;
        }
        unsigned int timestamp;
        memcpy(&timestamp, data, sizeof(unsigned int));
        for (int j = 0; j < MAX_RAID_DEVICES; j++)
        {
            if (timestamps[j] == 0 || timestamps[j] == timestamp)
            {
                timestamps[j] = timestamp;
                timestampsPosition[i] = j;
                break;
            }
        }

    }

    if (raid.countOfBrokenDisks > 2)
    {
        raid.status = RAID_FAILED;
        return;
    }

    int timestampsCount[MAX_RAID_DEVICES];
    for (int i = 0; i < MAX_RAID_DEVICES; i++)
    {
        timestampsCount[i] = 0;
        for (int j = 0; j < raid.devices; j++)
            if (timestampsPosition[j] == i)
                timestampsCount[i]++;
    }

    int maxRepresentation = 0;
    for (int i = 1; i < MAX_RAID_DEVICES; i++)
    {
        if (timestampsCount[i] > timestampsCount[maxRepresentation])
            maxRepresentation = i;
    }
    raid.timestamp = timestamps[maxRepresentation];

    for (int j = 0; j < raid.devices && raid.countOfBrokenDisks <= 2; j++)
        if (timestampsPosition[j] != maxRepresentation)
            raid.brokenDisks[raid.countOfBrokenDisks++] = j;


    if (raid.countOfBrokenDisks == 0)
    {
        raid.status = RAID_OK;
    }
    else if (raid.countOfBrokenDisks > 2)
    {
        raid.status = RAID_FAILED;
    }
    else
    {
        raid.status = RAID_DEGRADED;
    }
}

void RaidStop(void)
{
    using namespace Valkovic;

    if (raid.status == RAID_FAILED || raid.status == RAID_STOPPED)
    {
        raid.status = RAID_STOPPED;
        return;
    }

    raid.timestamp++;

    char data[SECTOR_SIZE];
    memcpy(data, &raid.timestamp, sizeof(unsigned int));

    for (int i = 0; i < raid.devices; i++)
    {
        if (raid.countOfBrokenDisks == 1 && raid.brokenDisks[0] == i)
            continue;
        if (raid.countOfBrokenDisks == 2 && (raid.brokenDisks[0] == i || raid.brokenDisks[1] == i))
            continue;

        raid.write(i, raid.sectors - 1, data, 1);
    }

    raid.status = RAID_STOPPED;
    return;
}

int RaidStatus(void)
{
    return Valkovic::raid.status;
}

int RaidSize(void)
{
    using namespace Valkovic;
    return (raid.sectors - 1) * (raid.devices - 2);
}

int RaidRead(int sector, void* d, int sectorCnt)
{
    using namespace Valkovic;

    if (raid.status == RAID_FAILED)
        return 0;

    char* data = (char*) d;

    int readed = 0;
    int workingSector = sector;
    int endSector = (sector + sectorCnt) > RaidSize() ? RaidSize() : (sector + sectorCnt);

    for (; workingSector < endSector && raid.status == RAID_OK;)
    {
        int line, column;
        raid.position(workingSector, column, line);

        if (raid.read(column, line, data, 1) == 1)
        {
            data += SECTOR_SIZE;
            workingSector++;
            readed++;
        }
    }

    for (; workingSector < endSector && raid.status == RAID_DEGRADED; workingSector++)
    {
        int line, column;
        raid.position(workingSector, column, line);

        conditions:
        if (raid.isNotBroken(column) && raid.read(column, line, data, 1) == 1)
        {
            data += SECTOR_SIZE;
            workingSector++;
            readed++;
        }

        if(raid.isBroken(column) && raid.countOfBrokenDisks == 1) //this is only damaged disk
        {
            int XORcolumn = line % raid.devices;
            int REEDcolumn = (XORcolumn+1) % raid.devices;

            char buffer[SECTOR_SIZE];
            memset(data,0,SECTOR_SIZE);

            for(int i=0;i<raid.devices && raid.status == RAID_DEGRADED;i++)
                if(i != REEDcolumn && i != column)
                {
                    int x = raid.read(i, line, buffer, 1);
                    if (x == 1)
                        for (int j = 0; j < SECTOR_SIZE; j++)
                            data[j] = data[j] ^ buffer[j];
                    else
                        goto conditions;
                }
            data += SECTOR_SIZE;
            workingSector++;
            readed++;
        }


        if(raid.isBroken(column) && raid.countOfBrokenDisks == 2) //this and another disk is broken
        {
            //TODO
        }
    }

    return readed;
}

int RaidWrite(int sector, const void* d, int sectorCnt)
{
    using namespace Valkovic;

    if (raid.status == RAID_FAILED)
        return 0;

    const char* data = (const char*) d;

    int wrote = 0;
    int workingSector = sector;
    int endSector = (sector + sectorCnt) > RaidSize() ? RaidSize() : (sector + sectorCnt);

    for (; workingSector < endSector && raid.status == RAID_OK;)
    {
        int line, column;
        raid.position(workingSector, column, line);

        char oldData[SECTOR_SIZE];
        if(raid.read(line % raid.devices,line,oldData,1) != 1)
        {
            //old data cannot be read, disk is broken
            continue;
        }
        if(raid.write(column,line,data,1) != 1)
        {
            //cannot write data to disk, disk is broken
            continue;
        }
        char xorBuffer[SECTOR_SIZE];
        //write to XOR disk
        if(raid.read(line % raid.devices,line,xorBuffer,1) != 1)
        {
            //cannot read from XOR disk, XOR disk is broken
            //ignore it, next loop will switch do DEGRADED
            data += SECTOR_SIZE;
            wrote++;
            workingSector++;
            continue;
        }

        //new XOR value = oldXOR ^ oldData ^ newData
        for(int i=0;i<SECTOR_SIZE;i++)
            xorBuffer[i] = xorBuffer[i] ^ oldData[i] ^ data[i];

        if(raid.write(line % raid.devices,line,xorBuffer,1) != 1)
        {
            //cannot write into XOR disk, XOR disk is broken
            //ignore it, next loop will swich to DEGRADED
            data += SECTOR_SIZE;
            wrote++;
            workingSector++;
            continue;
        }

        //TODO write to REED file

        data += SECTOR_SIZE;
        wrote++;
        workingSector++;
    }

    for (; workingSector < endSector && raid.status == RAID_DEGRADED; workingSector++)
    {
        int line, column;
        raid.position(workingSector, column, line);
        int xorDisk = line % raid.devices;
        int reedDisk = (xorDisk + 1) % raid.devices;

        char oldData[SECTOR_SIZE];
        bool oldReaded = false;
        if(raid.isNotBroken(column) && raid.isNotBroken(xorDisk) && raid.isNotBroken(reedDisk))
        {
            //Writable disk is OK
            if(raid.read(column,line,oldData,1) != 1)
            {
                //canot read from disk, broken
                continue;
            }
            oldReaded = true;
            if(raid.write(column,line,data,1) != 1)
            {
                //cannot write to disk, broken
                continue;
            }
        }
        else if(raid.isNotBroken(column) && raid.isNotBroken(xorDisk) && raid.isBroken(reedDisk))
        {

        }
        else if(raid.isNotBroken(column) && raid.isBroken(xorDisk) && raid.isNotBroken(reedDisk))
        {

        }
        else if(raid.isNotBroken(column) && raid.isBroken(xorDisk) && raid.isBroken(reedDisk))
        {

        }
        if(raid.isBroken(column) && raid.isNotBroken(xorDisk) && raid.isNotBroken(reedDisk))
        {

        }
        else if(raid.isBroken(column) && raid.isNotBroken(xorDisk) && raid.isBroken(reedDisk))
        {

        }
        else if(raid.isBroken(column) && raid.isBroken(xorDisk) && raid.isNotBroken(reedDisk))
        {

        }
        else if(raid.isBroken(column) && raid.isBroken(xorDisk) && raid.isBroken(reedDisk))
        {

        }
    }

    return wrote;
}

int RaidResync(void)
{
    return 255;
}