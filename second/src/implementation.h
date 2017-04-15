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
        int status;
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
            return this->_read(diskNumber, sectorNumber, data, sizeInSectors);
        }

        int write(int diskNumber, int sectorNumber, void* data, int sizeInSectors)
        {
            return this->_write(diskNumber, sectorNumber, data, sizeInSectors);
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
        if (timestampsPosition[j] != j)
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

int RaidRead(int sector, void* data, int sectorCnt)
{
    return 255;
}

int RaidWrite(int sector, const void* data, int sectorCnt)
{
    return 255;
}

int RaidResync(void)
{
    return 255;
}