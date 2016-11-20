/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLFile.h - EFI Native File Functions                            */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 12:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLFILE__
#define __SYSTEMLOADER_SLFILE__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

#define kSLFileModeCreate   0
#define kSLFileModeRead     1
#define kSLFileModeWrite    2

#if !kCXAssemblyCode

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
    SLABI SLStatus (*read)(OSAddress this, UIntN *size, OSAddress buffer);
    SLABI SLStatus (*write)(OSAddress this, UIntN *size, OSAddress buffer);
    SLABI SLStatus (*getOffset)(OSAddress this, UInt64 *offset);
    SLABI SLStatus (*setOffset)(OSAddress this, UInt64 offset);
    SLABI SLStatus (*getInfo)(OSAddress this, OSUIDIntelData *type, UIntN *size, OSAddress address);
    OSAddress unused2;
    SLABI SLStatus (*flush)(OSAddress this);
    OSAddress unused3[4];
} SLFile;

typedef struct {
    UInt64 revision;
    SLABI SLStatus (*openRoot)(OSAddress this, SLFile **root);
} SLVolume;

#if kCXBootloaderCode
    OSPrivate SLFile *SLGetRootDirectoryForImage(OSAddress imageHandle);

    OSPrivate SLFile *SLOpenChild(SLFile *parent, OSUTF8Char *child, UInt8 mode);
    OSPrivate SLFile *SLOpenPath(OSUTF8Char *path, UInt8 mode);
    OSPrivate bool SLCloseFile(SLFile *file);

    OSPrivate OSSize SLFileRead(SLFile *file, OSOffset offset, OSBuffer readBuffer);
    OSPrivate bool SLFileWrite(SLFile *file, OSBuffer writeBuffer);
    OSPrivate OSBuffer SLFileReadFully(SLFile *file);
    OSPrivate bool SLFileSync(SLFile *file);

    OSPrivate bool SLReadPath(OSUTF8Char *path, OSOffset offset, OSBuffer readBuffer);
    OSPrivate OSBuffer SLReadPathFully(OSUTF8Char *path);

    OSPrivate OSUTF16Char *SLPathToEFIPath(OSUTF8Char *path);
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_SLFILE__) */
