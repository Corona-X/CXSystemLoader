/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLFile.h - EFI Native File Functions                            */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 9.10.2016  - 12:00 AM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_EFI_SLFILE__
#define __SYSTEMLOADER_EFI_SLFILE__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

#define kSLFileModeCreate   0
#define kSLFileModeRead     1
#define kSLFileModeWrite    2

#if !kCXAssemblyCode

typedef struct {
    OSSize size;
    OSSize fileSize;
    UInt64 physicalSize;
    UInt64 createTime;
    UInt64 accessTime;
    UInt64 modTime;
    UInt64 attributes;
    OSUTF16Char name[];
} SLFileInfo;

typedef struct {
    UInt64 revision;
    SLABI SLStatus (*open)(OSAddress this, OSAddress *newFile, OSUTF16Char *filename, UInt64 mode, UInt64 attributes);
    SLABI SLStatus (*close)(OSAddress this);
    OSAddress delete;
    SLABI SLStatus (*read)(OSAddress this, OSSize *size, OSAddress buffer);
    OSAddress write;
    SLABI SLStatus (*getOffset)(OSAddress this, OSOffset *offset);
    SLABI SLStatus (*setOffset)(OSAddress this, OSOffset offset);
    SLABI SLStatus (*getInfo)(OSAddress this, OSUIDIntelData *type, OSSize *size, OSAddress address);
    OSAddress setInfo;
    SLABI SLStatus (*flush)(OSAddress this);
} SLFile;

typedef struct {
    UInt64 revision;
    SLABI SLStatus (*openRoot)(OSAddress this, SLFile **root);
} SLVolume;

#if kCXBootloaderCode
    OSPrivate SLFile *SLGetRootDirectoryForImage(OSAddress imageHandle);

    OSPrivate SLFile *SLOpenChild(SLFile *parent, const OSUTF16Char *child, UInt8 mode);
    OSPrivate SLFile *SLOpenPath(const OSUTF16Char *path, UInt8 mode);
    OSPrivate bool SLFileClose(SLFile *file);

    OSPrivate bool SLFileRead(SLFile *file, OSAddress buffer, OSSize size);
    OSPrivate bool SLPathRead(const OSUTF16Char *path, OSOffset offset, OSAddress buffer, OSSize size);
    OSPrivate OSOffset SLFileReadAt(SLFile *file, OSOffset offset, OSAddress buffer, OSSize size);

    OSPrivate bool SLFileSetOffset(SLFile *file, OSOffset offset);
    OSPrivate OSOffset SLFileGetOffset(SLFile *file);
    OSPrivate bool SLFileGetSize(SLFile *file, OSSize *size);
    OSPrivate bool SLPathGetSize(const OSUTF16Char *path, OSSize *size);

    OSPrivate OSAddress SLPathReadFully(const OSUTF16Char *path, OSSize *size);
    OSPrivate OSAddress SLFileReadFully(SLFile *file, OSSize *size);
    OSPrivate bool SLFileSync(SLFile *file);

    OSPrivate OSUTF16Char *SLPathToEFIPath(const OSUTF8Char *path);
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_SLFILE__) */
