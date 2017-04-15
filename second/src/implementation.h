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
        raidInstance(TBlkDev* dev) :
                _read(dev->m_Read), _write(dev->m_Write),
                devices(dev->m_Devices), sectors(dev->m_Sectors),
                status(RAID_OK), countOfBrokenDisks(0),
                brokenDisks{-1, -1}, timestamp(1)
        {}

        int devices;
        int sectors;
        int status;
        int countOfBrokenDisks;
        int brokenDisks[2];
        unsigned int timestamp;

        int read(int diskNumber, int sectorNumber, void* data, int sizeInSectors)
        {
            return this->_read(diskNumber, sectorNumber, data, sizeInSectors);
        }

        int write(int diskNumber, int sectorNumber, void* data, int sizeInSectors)
        {
            return this->_write(diskNumber, sectorNumber, data, sizeInSectors);
        }
    };

    raidInstance* raid = nullptr;
}

int RaidCreate(TBlkDev* dev)
{
    using namespace Valkovic;

    if (dev == nullptr)
        return 0;

    if (raid != nullptr)
        return 0;

    raid = new raidInstance(dev);

    char servisInformations[SECTOR_SIZE];
    memcpy(servisInformations, &raid->timestamp, sizeof(unsigned int));

    bool successfullyWrited = true;
    for (int i = 0; i < raid->devices && successfullyWrited; i++)
        successfullyWrited = (raid->write(i, raid->sectors - 1, servisInformations, 1) == 1);

    if (!successfullyWrited)
    {
        delete raid;
        raid = nullptr;
        return 0;
    }

    raid->status = RAID_STOPPED;
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
    return 255;
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