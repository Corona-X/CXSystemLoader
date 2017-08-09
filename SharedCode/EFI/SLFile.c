#include <SystemLoader/EFI/SLFile.h>
#include <SystemLoader/EFI/SLLoadedImage.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLibrary.h>

SLFile *SLGetRootDirectoryForImage(OSAddress imageHandle)
{
    if (kCXBuildDev) SLBootServicesCheck(kOSNullPointer);

    SLLoadedImage *loadedImage = SLLoadedImageGetFromHandle(imageHandle);
    if (!loadedImage) return kOSNullPointer;

    return SLLoadedImageGetRoot(loadedImage);
}

#pragma mark - Open/Close Functions

SLFile *SLOpenChild(SLFile *parent, const OSUTF8Char *child, UInt8 mode)
{
    SLBootServicesCheck(kOSNullPointer);
    SLFile *file;

    SLStatus status = parent->open(parent, &file, child, mode, 0);
    if (SLStatusIsError(status)) return kOSNullPointer;

    return file;
}

SLFile *SLOpenPath(const OSUTF8Char *path, UInt8 mode)
{
    if (kCXBuildDev) SLBootServicesCheck(kOSNullPointer);

    OSUTF16Char *efiPath = SLPathToEFIPath(path);
    if (!efiPath) return kOSNullPointer;

    SLFile *root = SLGetRootDirectoryForImage(SLGetMainImageHandle());
    if (!root) goto fail;

    SLFile *child = SLOpenChild(root, efiPath, mode);
    if (!child) goto fail;

    SLFree(efiPath);
    return child;

fail:

    SLFree(efiPath);
    return kOSNullPointer;
}

bool SLFileClose(SLFile *file)
{
    SLBootServicesCheck(false);

    SLStatus status = file->close(file);
    return !SLStatusIsError(status);
}

#pragma mark - Read Functions

bool SLFileRead(SLFile *file, OSAddress buffer, OSSize size)
{
    SLBootServicesCheck(false);
    OSSize readSize = size;

    SLStatus status = file->read(file, &readSize, buffer);
    if (readSize != size) return false;

    return !SLStatusIsError(status);
}

bool SLPathRead(const OSUTF8Char *path, OSOffset offset, OSAddress buffer, OSSize size)
{
    if (kCXBuildDev) SLBootServicesCheck(false);

    SLFile *file = SLOpenPath(path, kSLFileModeRead);
    if (!file) return false;

    bool success = SLFileReadAt(file, offset, buffer, size);
    return (SLFileClose(file) && success);
}

OSOffset SLFileReadAt(SLFile *file, OSOffset offset, OSAddress buffer, OSSize size)
{
    SLBootServicesCheck(-1);
    OSOffset initialOffset;

    SLStatus status = file->getOffset(file, &initialOffset);
    if (SLStatusIsError(status)) return -1;

    if (initialOffset != offset)
    {
        status = file->setOffset(file, offset);
        if (SLStatusIsError(status)) return -1;
    }

    if (!SLFileRead(file, buffer, size))
        return -1;

    return initialOffset;
}

#pragma mark - Write Functions

bool SLFileWrite(SLFile *file, OSAddress buffer, OSSize size)
{
    SLBootServicesCheck(false);
    OSSize writtenSize = size;

    SLStatus status = file->write(file, &writtenSize, buffer);
    if (writtenSize != size) return false;

    return !SLStatusIsError(status);
}

bool SLPathWrite(const OSUTF8Char *path, OSAddress buffer, OSSize size)
{
    if (kCXBuildDev) SLBootServicesCheck(false);

    SLFile *file = SLOpenPath(path, kSLFileModeWrite | kSLFileModeCreate);
    if (!file) return false;

    bool success = SLFileWrite(file, buffer, size);
    return (SLFileClose(file) && success);
}

OSOffset SLFileWriteAt(SLFile *file, OSOffset offset, OSAddress buffer, OSSize size)
{
    SLBootServicesCheck(-1);
    OSOffset initialOffset;

    SLStatus status = file->getOffset(file, &initialOffset);
    if (SLStatusIsError(status)) return -1;

    if (initialOffset != offset)
    {
        status = file->setOffset(file, offset);
        if (SLStatusIsError(status)) return -1;
    }

    if (!SLFileWrite(file, buffer, size))
        return -1;

    return initialOffset;
}

#pragma mark - Stream Manipulation/Statistics

bool SLFileSetOffset(SLFile *file, OSOffset offset)
{
    SLBootServicesCheck(false);

    SLStatus status = file->setOffset(file, offset);
    return !SLStatusIsError(status);
}

OSOffset SLFileGetOffset(SLFile *file)
{
    SLBootServicesCheck(false);
    OSOffset offset;

    SLStatus status = file->getOffset(file, &offset);
    if (SLStatusIsError(status)) return -1;

    return offset;
}

bool SLFileGetSize(SLFile *file, OSSize *size)
{
    SLBootServicesCheck(false);
    if (!size) return false;

    OSSize infoSize = sizeof(SLFileInfo);
    SLProtocol fileInfo = kSLFileInfoID;
    SLFileInfo info;

    SLStatus status = file->getInfo(file, &fileInfo, &infoSize, &info);
    if (status == kSLStatusBufferTooSmall) status = kSLStatusSuccess;
    if (SLStatusIsError(status)) return false;

    return info.fileSize;
}

bool SLPathGetSize(const OSUTF8Char *path, OSSize *size)
{
    if (kCXBuildDev) SLBootServicesCheck(false);

    SLFile *file = SLOpenPath(path, kSLFileModeWrite | kSLFileModeCreate);
    if (!file) return false;

    bool success = SLFileGetSize(file, size);
    return (SLFileClose(file) && success);
}

#pragma mark - Utility Functions

OSAddress SLPathReadFully(const OSUTF8Char *path, OSSize *size)
{
    SLBootServicesCheck(kOSNullPointer);

    SLFile *file = SLOpenPath(path, kSLFileModeRead);
    if (!file) return kOSNullPointer;

    OSAddress address = SLFileReadFully(file, &size);

    if (!SLFileClose(file))
    {
        if (address) SLFree(address);
        return kOSNullPointer;
    }

    if (!address) return kOSNullPointer;

    return address;
}

OSAddress SLFileReadFully(SLFile *file, OSSize *size)
{
    SLBootServicesCheck(kOSNullPointer);
    OSSize fileSize = 0;

    if (!SLFileGetSize(file, &fileSize))
        return kOSNullPointer;

    OSAddress address = SLAllocate(fileSize);
    if (!address) return kOSNullPointer;

    if (!SLFileSetOffset(file, 0))
        goto fail;

    if (!SLFileRead(file, address, fileSize))
        goto fail;

    if (size) (*size) = fileSize;
    return address;

fail:

    SLFree(address);
    return kOSNullPointer;
}

bool SLFileSync(SLFile *file)
{
    SLBootServicesCheck(false);

    SLStatus status = file->flush(file);
    return !SLStatusIsError(status);
}

#pragma mark - Path Conversion

OSUTF16Char *SLPathToEFIPath(const OSUTF8Char *path)
{
    return path;
}
