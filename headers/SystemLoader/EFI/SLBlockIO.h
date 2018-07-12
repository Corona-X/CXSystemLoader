/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLBlockIO.h - EFI Block I/O Protocol                            */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.8.2016   -  4:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_EFI_SLBLOCKIO__
#define __SYSTEMLOADER_EFI_SLBLOCKIO__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

#if !kCXAssemblyCode

typedef struct {
    UInt32 mediaID;
    bool removable;
    bool present;
    bool logicalPartition;
    bool readOnly;
    bool writeCaching;
    UInt32 blockSize;
    UInt32 ioAlign;
    OSOffset lastLBA;
    OSOffset lowestAlignedLBA;
    UInt32 logicalBlocksPerPhysicalBlock;
    UInt32 optimalTransferLength;
} SLBlockIOMedia;

typedef struct {
    UInt64 revision;
    SLBlockIOMedia *media;
    OSAddress reset;
    SLABI SLStatus (*readBlocks)(OSAddress this, UInt32 mediaID, OSOffset lba, OSSize size, OSAddress buffer);
    OSAddress writeBlocks;
    OSAddress synchronize;
} SLBlockIO;

#if kCXBootloaderCode
    SLBlockIO **SLBlockIOGetAll(OSCount *count);
    bool SLBlockIORead(SLBlockIO *object, OSOffset lba, OSAddress address, OSSize size);
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_EFI_SLBLOCKIO__) */
