#include <SystemLoader/EFI/SLLoadedImage.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/EFI/SLFile.h>
#include <SystemLoader/SLLibrary.h>

SLFile *SLGetRootDirectoryForImage(OSAddress imageHandle)
{
    if (kCXBuildDev) SLBootServicesCheck(kOSNullPointer);

    SLLoadedImage *loadedImage = SLLoadedImageGetFromHandle(imageHandle);
    if (!loadedImage) return kOSNullPointer;

    return SLLoadedImageGetRoot(loadedImage);
}

#pragma mark - Open/Close Functions

SLFile *SLOpenChild(SLFile *parent, const OSUTF16Char *child, UInt8 mode)
{
    SLBootServicesCheck(kOSNullPointer);
    SLFile *file;

    SLStatus status = parent->open(parent, &file, child, mode, 0);
    if (SLStatusIsError(status)) return kOSNullPointer;

    return file;
}

SLFile *SLOpenPath(const OSUTF16Char *path, UInt8 mode)
{
    if (kCXBuildDev) SLBootServicesCheck(kOSNullPointer);

    SLFile *root = SLGetRootDirectoryForImage(SLGetMainImageHandle());
    if (!root) return kOSNullPointer;

    SLFile *child = SLOpenChild(root, path, mode);
    if (!child) return kOSNullPointer;

    return child;
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

bool SLPathRead(const OSUTF16Char *path, OSOffset offset, OSAddress buffer, OSSize size)
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

    // UEFI is stupid and for some reason I can't just grab the pieces that I need
    // We have to allocate an array of the size they want for some reason and do it again
    // Otherwise it just refuses to give me anything, which is annoying...

    SLFileInfo *realInfo = SLAllocate(infoSize);
    if (!realInfo) return false;

    status = file->getInfo(file, &fileInfo, &infoSize, realInfo);
    if (SLStatusIsError(status)) return false;
    (*size) = realInfo->fileSize;

    SLFree(realInfo);
    return true;
}

bool SLPathGetSize(const OSUTF16Char *path, OSSize *size)
{
    if (kCXBuildDev) SLBootServicesCheck(false);

    SLFile *file = SLOpenPath(path, kSLFileModeWrite | kSLFileModeCreate);
    if (!file) return false;

    bool success = SLFileGetSize(file, size);
    return (SLFileClose(file) && success);
}

#pragma mark - Utility Functions

OSAddress SLPathReadFully(const OSUTF16Char *path, OSSize *size)
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

#pragma mark - Path Conversion

OSUTF16Char *SLPathToEFIPath(const OSUTF8Char *path)
{
    OSUTF8Char *string = path;
    OSLength length = 0;

    while (*string++)
        length++;

    OSUTF16Char *result = SLAllocate((length + 1) * sizeof(OSUTF16Char));

    if (!result)
        return kOSNullPointer;

    for(OSIndex i = 0; (OSLength)i < length; i++)
    {
        if (path[i] >= 0x80)
        {
            // This isn't supported here yet......
            SLFree(result);

            return kOSNullPointer;
        }

        if (path[i] == '/')
            result[i] = '\\';
        else
            result[i] = path[i];
    }

    return result;
}
