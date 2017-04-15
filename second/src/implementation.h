#ifndef __PROGTEST__
#include "definitions.h"
#endif

namespace Valkovic
{
    class raidInstance
    {
    public:
        int (* _read )(int, int, void*, int);
        int (* _write )(int, int, const void*, int);
    public:

        int devices;
        int sectors;
        int status = RAID_STOPPED;
        int brokenDisk;
        unsigned int timestamp;

        void getData(TBlkDev* dev)
        {
            _read = dev->m_Read;
            _write = dev->m_Write;
            devices = dev->m_Devices;
            sectors = dev->m_Sectors;
            status = RAID_OK;
            brokenDisk = -1;
            timestamp = 1;
        }

        int read(int diskNumber, int sectorNumber, void* data, int sizeInSectors)
        {
            if (brokenDisk == diskNumber)
                return 0;

            int readed = this->_read(diskNumber, sectorNumber, data, sizeInSectors);
            if (readed != sizeInSectors)
            {
                if (brokenDisk != -1)
                {
                    status = RAID_FAILED;
                }
                else
                {
                    brokenDisk = diskNumber;
                    status = RAID_DEGRADED;
                }
            };
            return readed;
        }

        int write(int diskNumber, int sectorNumber, const void* data, int sizeInSectors)
        {
            if (brokenDisk == diskNumber)
                return 0;

            int wrote = this->_write(diskNumber, sectorNumber, data, sizeInSectors);
            if (wrote != sizeInSectors)
            {
                if (brokenDisk != -1)
                {
                    status = RAID_FAILED;
                }
                else
                {
                    brokenDisk = diskNumber;
                    status = RAID_DEGRADED;
                }
            };
            return wrote;
        }

        inline bool isNotBroken(int disk) const
        {
            return brokenDisk != disk;
        }

        inline bool isBroken(int disk) const
        {
            return !isNotBroken(disk);
        }

        void position(int sector, int& column, int& line) const
        {
            line = sector / (devices - 1);
            int XORcolumn = line % devices;

            int sectorAtBeginOfLine = line * (devices - 1);
            int countOfSteps = sector - sectorAtBeginOfLine;
            if (XORcolumn <= countOfSteps)
                countOfSteps++;
            column = countOfSteps;
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
    for (int i = 0; i < raid.devices && countOfBroken <= 1; i++)
        if (raid.write(i, raid.sectors - 1, servisInformations, 1) != 1)
        {
            countOfBroken++;
            raid.brokenDisk = i;
        }

    if (countOfBroken > 1)
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

    for (int i = 0; i < raid.devices; i++)
    {
        timestampsPosition[i] = -1;
        if (raid.read(i, raid.sectors - 1, data, 1) != 1)
        {
            if (raid.status == RAID_FAILED)
                return;

            raid.brokenDisk = i;
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

    for (int j = 0; j < raid.devices; j++)
        if (timestampsPosition[j] != maxRepresentation)
        {
            if (raid.brokenDisk != -1 && raid.brokenDisk != j)
            {
                raid.status = RAID_FAILED;
                return;
            }
            raid.brokenDisk = j;
        }


    if (raid.brokenDisk == -1)
    {
        raid.status = RAID_OK;
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
        if (raid.brokenDisk == i)
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
    return (raid.sectors - 1) * (raid.devices - 1);
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

    for (; workingSector < endSector && raid.status == RAID_DEGRADED; )
    {
        int line, column;
        raid.position(workingSector, column, line);

        if (raid.isNotBroken(column) && raid.read(column, line, data, 1) == 1)
        { //other disk is broken
            data += SECTOR_SIZE;
            workingSector++;
            readed++;
        }
        else //disk is broken
        {
            memset(data, 0, SECTOR_SIZE);
            char buffer[SECTOR_SIZE];

            for (int i = 0; i < raid.devices; i++)
                if (i != raid.brokenDisk)
                {
                    raid.read(i, line, buffer, 1);
                    if (raid.status == RAID_FAILED)
                        return readed;
                    for (int j = 0; j < SECTOR_SIZE; j++)
                        data[j] = data[j] ^ buffer[j];
                }
            data += SECTOR_SIZE;
            workingSector++;
            readed++;
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
        int xorDisk = line % raid.devices;


        char oldData[SECTOR_SIZE];
        if (raid.read(column, line, oldData, 1) != 1)
        {
            //old data cannot be read, disk is broken
            continue;
        }
        if (raid.write(column, line, data, 1) != 1)
        {
            //cannot write data to disk, disk is broken
            continue;
        }
        data += SECTOR_SIZE;
        wrote++;
        workingSector++;

        //SOLVE XOR
        char xorBuffer[SECTOR_SIZE];
        //write to XOR disk
        if (raid.read(xorDisk, line, xorBuffer, 1) != 1)
        {
            //cannot read from XOR disk, XOR disk is broken
            //ignore it, next loop will switch do DEGRADED
            continue;
        }
        //new XOR value = oldXOR ^ oldData ^ newData
        for (int i = 0; i < SECTOR_SIZE; i++)
            xorBuffer[i] = xorBuffer[i] ^ oldData[i] ^ data[i-SECTOR_SIZE];

        if (raid.write(xorDisk, line, xorBuffer, 1) != 1)
        {
            //cannot write into XOR disk, XOR disk is broken
            //ignore it, next loop will swich to DEGRADED
            continue;
        }
    }

    for (; workingSector < endSector && raid.status == RAID_DEGRADED;)
    {
        int line, column;
        raid.position(workingSector, column, line);
        int xorDisk = line % raid.devices;

        if (raid.isNotBroken(column))
        {
            char oldData[SECTOR_SIZE];
            if (raid.read(column, line, oldData, 1) != 1)
            {
                //old data cannot be read, disk is broken
                return wrote;
            }
            if (raid.write(column, line, data, 1) != 1)
            {
                //cannot write data to disk, disk is broken
                return wrote;
            }

            data += SECTOR_SIZE;
            wrote++;
            workingSector++;

            if (raid.isNotBroken(xorDisk))
            {
                //SOLVE XOR
                char xorBuffer[SECTOR_SIZE];
                //write to XOR disk
                if (raid.read(xorDisk, line, xorBuffer, 1) != 1)
                {
                    //cannot read from XOR disk, XOR disk is broken
                    return wrote;
                }
                for (int i = 0; i < SECTOR_SIZE; i++)
                    xorBuffer[i] = xorBuffer[i] ^ oldData[i] ^ data[i-SECTOR_SIZE];

                if (raid.write(xorDisk, line, xorBuffer, 1) != 1)
                {
                    //cannot write into XOR disk, XOR disk is broken
                    return wrote;
                }
            }
            else
            {
                //XOR disk is broken, ignore it
                continue;
            }
        }
        else //disk is broken
        {
            char xored[SECTOR_SIZE];
            memset(xored, 0, SECTOR_SIZE);
            char buffer[SECTOR_SIZE];

            //write only to XOR disk
            for (int i = 0; i < raid.devices; i++)
                if (i != xorDisk && i != column) //i != column je ekvivalentni i == brokenDisk
                {
                    raid.read(i, line, buffer, 1);
                    if (raid.status == RAID_FAILED)
                        return wrote;

                    for (int j = 0; j < SECTOR_SIZE; j++)
                        xored[j] = xored[j] ^ buffer[j];
                }

            for (int j = 0; j < SECTOR_SIZE; j++)
                xored[j] = xored[j] ^ data[j];

            if (raid.write(xorDisk, line, xored, 1) != 1)
            {
                return wrote;
            }

            data += SECTOR_SIZE;
            wrote++;
            workingSector++;
        }
    }

    return wrote;
}

int RaidResync(void)
{
    using namespace Valkovic;

    if(raid.status == RAID_OK || raid.status == RAID_STOPPED || raid.status== RAID_FAILED)
        return 0;

    for(int i=0;i<raid.sectors;i++)
    {
        char writeBuffer[SECTOR_SIZE];
        char readBuffer[SECTOR_SIZE];
        memset(writeBuffer,0,SECTOR_SIZE);

        //read data
        for(int j=0;j<raid.devices;j++)
        {
            if(j==raid.brokenDisk)
                continue;

            if(raid._read(j,i,readBuffer,1) != 1)
            {
                //read from another disk failed
                raid.status = RAID_FAILED;
                return 0;
            }

            for(int k=0;k<SECTOR_SIZE;k++)
                writeBuffer[k] = writeBuffer[k] ^ readBuffer[k];
        }

        //write data
        if(raid._write(raid.brokenDisk,i,writeBuffer,1) != 1)
        {
            raid.status = RAID_DEGRADED;
            return 0;
        }
    }

    raid.status = RAID_OK;
    return 1;
}