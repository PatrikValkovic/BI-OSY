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
                devices(dev->m_Devices), sectors(dev->m_Sectors),
                _read(dev->m_Read), _write(dev->m_Write), status(RAID_OK)
        { }

        int status;
        int devices;
        int sectors;
        int read(int diskNumber, int sectorNumber, void* data, int sizeInSectors)
        {
            return this->_read(diskNumber,sectorNumber, data,sizeInSectors);
        }
        int write(int diskNumber, int sectorNumber, void* data, int sizeInSectors)
        {
            return this->_write(diskNumber,sectorNumber, data,sizeInSectors);
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
    unsigned int counter = 0;
    memcpy(servisInformations, &counter, sizeof(unsigned int));

    bool successfullyWrited = true;
    for(int i=0;i<raid->devices && successfullyWrited;i++)
        successfullyWrited = (raid->write(i,raid->sectors-1,servisInformations,1) == 1);

    if(!successfullyWrited)
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

}

int RaidSize(void)
{

}

int RaidRead(int sector, void* data, int sectorCnt)
{

}

int RaidWrite(int sector, const void* data, int sectorCnt)
{

}

int RaidResync(void)
{

}