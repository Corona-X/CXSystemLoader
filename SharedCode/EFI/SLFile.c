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

SLFile *SLOpenChild(SLFile *parent, OSUTF8Char *child, UInt8 mode)
{
    SLBootServicesCheck(kOSNullPointer);
    SLFile *file;

    SLStatus status = parent->open(parent, &file, child, mode, 0);
    if (SLStatusIsError(status)) return kOSNullPointer;

    return file;
}

SLFile *SLOpenPath(OSUTF8Char *path, UInt8 mode)
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

bool SLPathRead(OSUTF8Char *path, OSOffset offset, OSAddress buffer, OSSize size)
{
    if (kCXBuildDev) SLBootServicesCheck(false);

    SLFile *file = SLOpenPath(path, kSLFileModeRead);
    if (!file) return false;

    bool success = SLFileReadAt(file, offset, buffer, size);
    return (SLFileClose(file) && success);
}

OSOffset SLFileReadAt(SLFile *file, OSOffset offset, OSAddress buffer, OSSize size)
{
    SLBootServicesCheck(false);
    OSOffset initialOffset;

    SLStatus status = file->getOffset(file, &initialOffset);
    if (SLStatusIsError(status)) return false;

    if (initialOffset != offset)
    {
        status = file->setOffset(file, offset);
        if (SLStatusIsError(status)) return false;
    }

    if (!SLFileRead(file, buffer, size))
        return ~((OSOffset)0);

    return initialOffset;
}

#pragma mark - Write Functions

#pragma mark - Stream Manipulation/Statistics

#pragma mark - Utility Functions

/*OSBuffer SLFileReadFully(SLFile *file)
{
    SLBootServicesCheck(kOSBufferEmpty);
    
    OSUIDIntelData fileInfoUID = kSLFileInfoID;
    OSSize fileInfoSize = sizeof(SLFileInfo);
    SLFileInfo fileInfo;
    
    SLStatus status = file->getInfo(file, &fileInfoUID, &fileInfoSize, &fileInfo);
    if (status == kSLStatusBufferTooSmall) status = kSLStatusSuccess;
    if (SLStatusIsError(status)) return kOSBufferEmpty;
    
    OSBuffer buffer = SLAllocate(fileInfo.size);
    if (OSBufferIsEmpty(buffer)) return kOSBufferEmpty;
    status = file->read(file, &buffer.size, buffer.address);
    
    if (SLStatusIsError(status) || buffer.size < fileInfo.size)
    {
        SLFreeBuffer(buffer);
        
        buffer = kOSBufferEmpty;
    }
    
    return buffer;
}

OSBuffer SLReadPathFully(OSUTF8Char *path)
{
    SLBootServicesCheck(kOSBufferEmpty);

    SLFile *file = SLOpenPath(path, kSLFileModeRead);
    if (!file) return kOSBufferEmpty;

    OSBuffer result = SLFileReadFully(path);

    if (!SLCloseFile(file) && !OSBufferIsEmpty(result))
    {
        SLFreeBuffer(result);

        result = kOSBufferEmpty;
    }

    return result;
}*/

#pragma mark - Path Conversion

OSUTF16Char *SLPathToEFIPath(OSUTF8Char *path)
{
    return path;
}
