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
    UInt64 unused1;
    OSSize size;
    UInt64 unused2[4];
    UInt16 *name;
} SLFileInfo;

typedef struct {
    UInt64 revision;
    SLABI SLStatus (*open)(OSAddress this, OSAddress *newFile, OSUTF16Char *filename, UInt64 mode, UInt64 attributes);
    SLABI SLStatus (*close)(OSAddress this);
    OSAddress delete;
    SLABI SLStatus (*read)(OSAddress this, OSSize *size, OSAddress buffer);
    SLABI SLStatus (*write)(OSAddress this, OSSize *size, OSAddress buffer);
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

    OSPrivate SLFile *SLOpenChild(SLFile *parent, OSUTF8Char *child, UInt8 mode);
    OSPrivate SLFile *SLOpenPath(OSUTF8Char *path, UInt8 mode);
    OSPrivate bool SLFileClose(SLFile *file);

    OSPrivate bool SLFileRead(SLFile *file, OSAddress buffer, OSSize size);
    OSPrivate bool SLPathRead(OSUTF8Char *path, OSOffset offset, OSAddress buffer, OSSize size);
    OSPrivate OSOffset SLFileReadAt(SLFile *file, OSOffset offset, OSAddress buffer, OSSize size);

    OSPrivate bool SLFileWrite(SLFile *file, OSAddress buffer, OSSize size);
    OSPrivate bool SLPathWrite(OSUTF8Char *path, OSAddress buffer, OSSize size);
    OSPrivate OSOffset SLFileWriteAt(SLFile *file, OSOffset offset, OSAddress buffer, OSSize size);

    OSPrivate bool SLFileSetOffset(SLFile *file, OSOffset offset);
    OSPrivate bool SLFileGetOffset(SLFile *file, OSOffset offset);
    OSPrivate bool SLFileGetSize(SLFile *file, OSSize *size);
    OSPrivate bool SLPathGetSize(SLFile *file, OSSize *size);

    OSPrivate OSAddress SLPathReadFully(OSUTF8Char *path, OSSize *size);
    OSPrivate OSAddress SLFileReadFully(SLFile *file, OSSize *size);
    OSPrivate bool SLFileSync(SLFile *file);

    OSPrivate OSUTF16Char *SLPathToEFIPath(OSUTF8Char *path);
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_SLFILE__) */
