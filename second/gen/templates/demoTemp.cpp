class Disk
{
public:
    unsigned char buffer[SECTOR_SIZE * {{countOfSectors}}];
    bool working;
    Disk()
    {
        memset(buffer,0,SECTOR_SIZE * {{countOfSectors}});
        working = true;
    }
};

int outerWrite(int disk, int sector, const void* d, int sectorCnt);
int outerRead(int disk, int sector, void* d, int sectorCnt);

class manager
{
public:
    TBlkDev d;

    int innerWrite(int disk, int sector, const void* d, int sectorCnt)
    {
        if (!this->disks[disk].working)
            return 0;
        memcpy(this->disks[disk].buffer + (sector * SECTOR_SIZE), d, (size_t) sectorCnt * SECTOR_SIZE);
        return sectorCnt;
    };

    int innerRead(int disk, int sector, void* d, int sectorCnt)
    {
        if (!this->disks[disk].working)
            return 0;
        memcpy(d, this->disks[disk].buffer + (sector * SECTOR_SIZE), (size_t) sectorCnt * SECTOR_SIZE);
        return sectorCnt;
    };

public:
    Disk disks[{{countOfDisks}}];

    TBlkDev* getDev()
    {
        d.m_Sectors = {{countOfSectors}};
        d.m_Devices = {{countOfDisks}};
        d.m_Write = outerWrite;
        d.m_Read = outerRead;
        return &d;
    }

    void disableDisk(int disk)
    {
        this->disks[disk].working = false;
    }
};

manager* currentInstance;

int outerWrite(int disk, int sector, const void* d, int sectorCnt)
{
    return currentInstance->innerWrite(disk,sector,d,sectorCnt);
}
int outerRead(int disk, int sector, void* d, int sectorCnt)
{
    return currentInstance->innerRead(disk,sector,d,sectorCnt);
}

void test()
{
    {{test_text}}
}