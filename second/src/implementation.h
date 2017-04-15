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
    for (int i = 0; i < raid.devices && countOfBroken < 2; i++)
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

}

void RaidStop(void)
{

}

int RaidStatus(void)
{
    return Valkovic::raid.status;
}

int RaidSize(void)
{
    return 255;
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