/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLFile.h - EFI Native File Functions                            */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 12:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 12:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLFILE__
#define __SYSTEMLOADER_SLFILE__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

#define kSLFileModeRead     1

typedef struct {
    UInt64 unused1;
    UInt64 size;
    UInt64 unused2[4];
    UInt16 *name;
} SLFileInfo;

typedef struct {
    UInt64 revision;
    SLABI SLStatus (*open)(OSAddress this, OSAddress *newFile, OSUTF16Char *filename, UInt64 mode, UInt64 attributes);
    SLABI SLStatus (*close)(OSAddress this);
    OSAddress unused1;
    SLABI SLStatus (*read)(OSAddress this, UIntN *size, OSAddress address);
    OSAddress unused2;
    SLABI SLStatus (*getOffset)(OSAddress this, UInt64 *offset);
    SLABI SLStatus (*setOffset)(OSAddress this, UInt64 offset);
    SLABI SLStatus (*getInfo)(OSAddress this, OSUIDIntelData *type, UIntN *size, OSAddress address);
    OSAddress unused3[6];
} SLFile;

typedef struct {
    UInt64 revision;
    SLABI SLStatus (*openRoot)(OSAddress this, SLFile **root);
} SLVolume;

#if kCXBootloaderCode
    OSPrivate SLFile *SLGetRootDirectoryForImage(OSAddress imageHandle);
    OSPrivate SLFile *SLOpenChild(SLFile *parent, OSUTF8Char *child);
    OSPrivate SLFile *SLOpenPath(OSUTF8Char *path);
    OSPrivate bool SLCloseFile(SLFile *file);

    OSPrivate OSSize SLFileRead(SLFile *file, OSOffset offset, OSBuffer readBuffer);
    OSPrivate bool SLFileWrite();
    OSPrivate OSBuffer SLFileReadFully(SLFile *file);

    OSPrivate bool SLReadPath(OSUTF8Char *path, OSOffset offset, OSBuffer readBuffer);
    OSPrivate OSBuffer SLReadPathFully(OSUTF8Char *path);

    OSPrivate OSUTF16Char *SLPathToEFIPath(OSUTF16Char *path);
#endif /* kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_SLFILE__) */
