#include "implementation.h"

/* SW RAID6 - basic test
 *
 * The testing of the RAID driver requires a backend (simulating the underlying disks).
 * Next, the tests of your RAID implemetnation are needed. To help you with the implementation,
 * a sample backend is implemented in this file. It provides a quick-and-dirty
 * implementation of the underlying disks (simulated in files) and a few Raid... function calls.
 *
 * The implementation in the real testing environment is different. The sample below is a
 * minimalistic disk backend which matches the required interface. The backend, for instance,
 * cannot simulate a crashed disk. To test your Raid implementation, you will have to modify
 * or extend the backend.
 *
 * Next, you will have to add some raid testing. There is a few Raid... functions called from within
 * main(), however, the tests are incomplete. For instance, RaidResync () is not tested here. Once
 * again, this is only a starting point.
 */


#define RAID_DEVICES 4
#define DISK_SECTORS 8192

static int blocked = -1;

static FILE  * g_Fp[RAID_DEVICES];

//-------------------------------------------------------------------------------------------------
/** Sample sector reading function. The function will be called by your Raid driver implementation.
 * Notice, the function is not called directly. Instead, the function will be invoked indirectly
 * through function pointer in the TBlkDev structure.
 */
int                diskRead                                ( int               device,
                                                             int               sectorNr,
                                                             void            * data,
                                                             int               sectorCnt )
{
    if(device == blocked) //TODO my
        return 0;

    if ( device < 0 || device >= RAID_DEVICES )
        return 0;
    if ( g_Fp[device] == NULL )
        return 0;
    if ( sectorCnt <= 0 || sectorNr + sectorCnt > DISK_SECTORS )
        return 0;
    fseek ( g_Fp[device], sectorNr * SECTOR_SIZE, SEEK_SET );
    return fread ( data, SECTOR_SIZE, sectorCnt, g_Fp[device] );
}
//-------------------------------------------------------------------------------------------------
/** Sample sector writing function. Similar to diskRead
 */
int                diskWrite                               ( int               device,
                                                             int               sectorNr,
                                                             const void      * data,
                                                             int               sectorCnt )
{
    if(device == blocked) //TODO my
        return 0;


    if ( device < 0 || device >= RAID_DEVICES )
        return 0;
    if ( g_Fp[device] == NULL )
        return 0;
    if ( sectorCnt <= 0 || sectorNr + sectorCnt > DISK_SECTORS )
        return 0;
    fseek ( g_Fp[device], sectorNr * SECTOR_SIZE, SEEK_SET );
    return fwrite ( data, SECTOR_SIZE, sectorCnt, g_Fp[device] );
}
//-------------------------------------------------------------------------------------------------
/** A function which releases resources allocated by openDisks/createDisks
 */
void               doneDisks                               ( TBlkDev         * dev )
{
    delete dev;
    for ( int i = 0; i < RAID_DEVICES; i ++ )
        if ( g_Fp[i] )
        {
            fclose ( g_Fp[i] );
            g_Fp[i]  = NULL;
        }
}
//-------------------------------------------------------------------------------------------------
/** A function which creates the files needed for the sector reading/writing functions above.
 * This function is only needed for the particular implementation above.
 */
TBlkDev          * createDisks                             ( void )
{
    char       buffer[SECTOR_SIZE];
    TBlkDev  * res = NULL;
    char       fn[100];

    memset    ( buffer, 0, sizeof ( buffer ) );

    for ( int i = 0; i < RAID_DEVICES; i ++ )
    {
        snprintf ( fn, sizeof ( fn ), "/tmp/%04d", i );
        g_Fp[i] = fopen ( fn, "w+b" );
        if ( ! g_Fp[i] )
        {
            doneDisks ( NULL );
            return NULL;
        }

        for ( int j = 0; j < DISK_SECTORS; j ++ )
            if ( fwrite ( buffer, sizeof ( buffer ), 1, g_Fp[i] ) != 1 )
            {
                doneDisks ( NULL );
                return NULL;
            }
    }

    res              = new TBlkDev;
    res -> m_Devices = RAID_DEVICES;
    res -> m_Sectors = DISK_SECTORS;
    res -> m_Read    = diskRead;
    res -> m_Write   = diskWrite;
    return res;
}
//-------------------------------------------------------------------------------------------------
/** A function which opens the files needed for the sector reading/writing functions above.
 * This function is only needed for the particular implementation above.
 */
TBlkDev          * openDisks                               ( void )
{
    TBlkDev  * res = NULL;
    char       fn[100];

    for ( int i = 0; i < RAID_DEVICES; i ++ )
    {
        snprintf ( fn, sizeof ( fn ), "/tmp/%04d", i );
        g_Fp[i] = fopen ( fn, "r+b" );
        if ( ! g_Fp[i] )
        {
            doneDisks ( NULL );
            return NULL;
        }
        fseek ( g_Fp[i], 0, SEEK_END );
        if ( ftell ( g_Fp[i] ) != DISK_SECTORS * SECTOR_SIZE )
        {
            doneDisks ( NULL );
            return NULL;
        }
    }

    res              = new TBlkDev;
    res -> m_Devices = RAID_DEVICES;
    res -> m_Sectors = DISK_SECTORS;
    res -> m_Read    = diskRead;
    res -> m_Write   = diskWrite;
    return res;
}

void test1()
{
    int i;
    int retCode;
    bool status = true;

    cout << "Test 1 (writing and reading to all sector by one sector): ";
    for ( i = 0; i < RaidSize (); i ++ )
    {
        char buffer [SECTOR_SIZE];

        retCode = RaidWrite ( i, buffer, 1 );
        retCode = RaidRead ( i, buffer, 1 );
    }

    if(RaidStatus() != RAID_DEGRADED)
        cout << "FAILED" << endl;
    else
        cout << "OK" << endl;

    cout << "Test 2 (writing and reading to almost all sector by many sectors): ";

    for ( i = 0; i < RaidSize (); i += 10 ){
        char buffer2 [10 * SECTOR_SIZE];
        memset(&buffer2, i, 10 * SECTOR_SIZE);

        retCode = RaidWrite ( i, buffer2, 10 );

    }

    char tempBuffer[10 * SECTOR_SIZE];
    for ( i = 0; i < RaidSize (); i += 10 ){
        char buffer3 [10 * SECTOR_SIZE];
        memset(&tempBuffer, i, 10 * SECTOR_SIZE);
        retCode = RaidRead ( i, buffer3, 10 );
        if(memcmp(tempBuffer, buffer3, retCode * SECTOR_SIZE) != 0){
            status = false;
            break;
        }
    }
    if(RaidStatus() != RAID_DEGRADED || !status)
        cout << "FAILED" << endl;
    else
        cout << "OK" << endl;


    cout << "Test 3 (writing and reading to random sectors by various count of sectors): ";
    char buffer4 [5 * SECTOR_SIZE];
    char buffer5 [6 * SECTOR_SIZE];

    memset(&buffer4, 4, 5 * SECTOR_SIZE);
    memset(&buffer5, 5, 6 * SECTOR_SIZE);

    retCode = RaidWrite ( 2000, buffer4, 5 );
    retCode += RaidWrite ( 3000, buffer4, 5 );
    retCode += RaidWrite ( 4000, buffer4, 5 );
    retCode += RaidWrite ( 5000, buffer4, 5 );
    retCode += RaidWrite ( 6000, buffer4, 5 );
    retCode += RaidWrite ( 7000, buffer4, 5 );
    retCode += RaidWrite ( 8000, buffer4, 5 );
    retCode += RaidWrite ( 9000, buffer4, 5 );
    retCode += RaidWrite ( 10000, buffer4, 5 );
    retCode += RaidWrite ( 12000, buffer4, 5 );

    retCode += RaidWrite ( 124, buffer5, 6 );
    retCode += RaidWrite ( 5443, buffer5, 6 );
    retCode += RaidWrite ( 1, buffer5, 6 );
    retCode += RaidWrite ( 1432, buffer5, 6 );
    retCode += RaidWrite ( 23000, buffer5, 6 );
    retCode += RaidWrite ( 3333, buffer5, 6 );
    retCode += RaidWrite ( 18000, buffer5, 6 );
    retCode += RaidWrite ( 12876, buffer5, 6 );
    retCode += RaidWrite ( 9787, buffer5, 6 );
    retCode += RaidWrite ( 24565, buffer5, 6 );

    if(retCode != 110)
        cout << "return count mishmash, ";
    char tempBuffer4[5 * SECTOR_SIZE];
    char tempBuffer5[6 * SECTOR_SIZE];

    retCode = RaidRead ( 2000, tempBuffer4, 5 );
    if(memcmp(&buffer4, &tempBuffer4, 5 * SECTOR_SIZE) != 0)
        status = false;
    retCode += RaidRead ( 3000, tempBuffer4, 5 );
    if(memcmp(&buffer4, &tempBuffer4, 5 * SECTOR_SIZE) != 0)
        status = false;
    retCode += RaidRead ( 4000, tempBuffer4, 5 );
    if(memcmp(&buffer4, &tempBuffer4, 5 * SECTOR_SIZE) != 0)
        status = false;
    retCode += RaidRead ( 5000, tempBuffer4, 5 );
    if(memcmp(&buffer4, &tempBuffer4, 5 * SECTOR_SIZE) != 0)
        status = false;
    retCode += RaidRead ( 6000, tempBuffer4, 5 );
    if(memcmp(&buffer4, &tempBuffer4, 5 * SECTOR_SIZE) != 0)
        status = false;
    retCode += RaidRead ( 7000, tempBuffer4, 5 );
    if(memcmp(&buffer4, &tempBuffer4, 5 * SECTOR_SIZE) != 0)
        status = false;
    retCode += RaidRead ( 8000, tempBuffer4, 5 );
    if(memcmp(&buffer4, &tempBuffer4, 5 * SECTOR_SIZE) != 0)
        status = false;
    retCode += RaidRead ( 9000, tempBuffer4, 5 );
    if(memcmp(&buffer4, &tempBuffer4, 5 * SECTOR_SIZE) != 0)
        status = false;
    retCode += RaidRead ( 10000, tempBuffer4, 5 );
    if(memcmp(&buffer4, &tempBuffer4, 5 * SECTOR_SIZE) != 0)
        status = false;
    retCode += RaidRead ( 12000, tempBuffer4, 5 );
    if(memcmp(&buffer4, &tempBuffer4, 5 * SECTOR_SIZE) != 0)
        status = false;

    retCode += RaidRead ( 124, tempBuffer5, 6 );
    if(memcmp(&buffer5, &tempBuffer5, 6 * SECTOR_SIZE) != 0)
        status = false;
    retCode += RaidRead ( 5443, tempBuffer5, 6 );
    if(memcmp(&buffer5, &tempBuffer5, 6 * SECTOR_SIZE) != 0)
        status = false;
    retCode += RaidRead ( 1, tempBuffer5, 6 );
    if(memcmp(&buffer5, &tempBuffer5, 6 * SECTOR_SIZE) != 0)
        status = false;
    retCode += RaidRead ( 1432, tempBuffer5, 6 );
    if(memcmp(&buffer5, &tempBuffer5, 6 * SECTOR_SIZE) != 0)
        status = false;
    retCode += RaidRead ( 23000, tempBuffer5, 6 );
    if(memcmp(&buffer5, &tempBuffer5, 6 * SECTOR_SIZE) != 0)
        status = false;
    retCode += RaidRead ( 3333, tempBuffer5, 6 );
    if(memcmp(&buffer5, &tempBuffer5, 6 * SECTOR_SIZE) != 0)
        status = false;
    retCode += RaidRead ( 18000, tempBuffer5, 6 );
    if(memcmp(&buffer5, &tempBuffer5, 6 * SECTOR_SIZE) != 0)
        status = false;
    retCode += RaidRead ( 12876, tempBuffer5, 6 );
    if(memcmp(&buffer5, &tempBuffer5, 6 * SECTOR_SIZE) != 0)
        status = false;
    retCode += RaidRead ( 9787, tempBuffer5, 6 );
    if(memcmp(&buffer5, &tempBuffer5, 6 * SECTOR_SIZE) != 0)
        status = false;
    retCode += RaidRead ( 24565, tempBuffer5, 6 );
    if(memcmp(&buffer5, &tempBuffer5, 6 * SECTOR_SIZE) != 0)
        status = false;
    if(retCode != 110)
        cout << "return count mishmash, ";

    if(RaidStatus() != RAID_DEGRADED || !status)
        cout << "FAILED" << endl;
    else

        cout << "OK" << endl;

}

//-------------------------------------------------------------------------------------------------
int main ( void )
{
    blocked = 1;

    assert(RaidCreate(NULL) == 0);
    assert(RaidCreate(nullptr) == 0);

    /* create the disks before we use them
     */
    TBlkDev * dev = createDisks ();
    /* The disks are ready at this moment. Your RAID-related functions may be executed,
     * the disk backend is ready.
     *
     * First, try to create the RAID:
     */

    int retCode = RaidCreate ( dev );
    assert ( retCode == 1 );

    /* start it */

    RaidStart ( dev );

    assert(RaidStatus() == RAID_DEGRADED);

    /* your raid device shall be up.
     * try to read and write all RAID sectors:
     */

    for ( int i = 0; i < RaidSize (); i ++ )
    {
        char buffer1 [SECTOR_SIZE] = {'a','b','c','d'};
        char buffer2 [SECTOR_SIZE] = {'x', 'y', 'y', 'z'};

        retCode = RaidWrite ( i, buffer1, 1 );
        assert(retCode == 1);
        retCode = RaidRead ( i, buffer2, 1 );
        assert(retCode == 1);
        assert(memcmp(buffer1,buffer2,SECTOR_SIZE)==0);

    }

    test1();

    /* Extensive testing of your RAID implementation ...
     */

    blocked = -1;

    assert(RaidResync()==1);

    /* Stop the raid device ...
     */
    RaidStop ();

    /* ... and the underlying disks.
     */

    doneDisks ( dev );

    /* The RAID as well as disks are stopped. It corresponds i.e. to the
     * restart of a real computer.
     *
     * after the restart, we will not create the disks, nor create RAID (we do not
     * want to destroy the content). Instead, we will only open/start the devices:
     */

    dev = openDisks ();
    RaidStart ( dev );

    /* some I/O: RaidRead/RaidWrite
     */

    RaidStop ();
    doneDisks ( dev );

    return 0;
}

