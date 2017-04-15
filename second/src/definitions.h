#ifndef __PROGTEST__
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cstdint>
#include <cassert>
#include <iostream>

using namespace std;

#define SECTOR_SIZE                     512
#define MAX_RAID_DEVICES                 16
#define MAX_DEVICE_SECTORS (1024 * 1024 * 2)
#define MIN_DEVICE_SECTORS (   1 * 1024 * 2)

#define RAID_OK                           0
#define RAID_DEGRADED                     1
#define RAID_FAILED                       2
#define RAID_STOPPED                      3

struct TBlkDev
{
    int              m_Devices;
    int              m_Sectors;
    int           (* m_Read )  ( int, int, void *, int );
    int           (* m_Write ) ( int, int, const void *, int );
};
#endif /* __PROGTEST__ */