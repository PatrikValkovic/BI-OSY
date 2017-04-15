class Disk
{
public:
    unsigned char buffer[SECTOR_SIZE * {{countOfSectors}}];
    bool working;
    Disk()
    {
        memset(buffer,0,SECTOR_SIZE * {{countOfSectors}}];
        working = true;
    }
};

class manager
{
private:
    TBlkDev d;
public:
    Disk disks[{{countOfDisks}}];

    TBlkDev* getDev()
    {
        d.m_Sectors = {{countOfSectors}};
        d.m_Devices = {{countOfDisks}};
        d.m_Write = [this](int disk, int sector, const void* d, int sectorCnt)->int{
            if(!this->disks[disk].working)
                return 0;
            memcpy(this->disks[disk].buffer+(sector * SECTOR_SIZE),d,(size_t)sectorCnt*SECTOR_SIZE);
            return sectorCnt;
        };
        d.m_Read = [this](int disk, int sector, void* d, int sectorCnt)->int{
            if(!this->disks[disk].working)
                return 0;
            memcpy(d,this->disks[disk].buffer+(sector * SECTOR_SIZE),(size_t)sectorCnt*SECTOR_SIZE);
            return sectorCnt;
        };
        return &d;
    }

    void disableDisk(int disk)
    {
        this->disks[disk].working = false;
    }
};