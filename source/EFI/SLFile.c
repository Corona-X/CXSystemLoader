#include <SystemLoader/EFI/SLFile.h>
#include <SystemLoader/EFI/SLLoadedImage.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLibrary.h>

SLFile *SLGetRootDirectoryForImage(OSAddress imageHandle)
{
    SLBootServicesCheck(kOSNullPointer);

    SLLoadedImage *loadedImage = SLLoadedImageGetFromHandle(imageHandle);
    if (!loadedImage) return kOSNullPointer;

    return SLLoadedImageGetRoot(loadedImage);
}

SLFile *SLOpenChild(SLFile *parent, OSUTF8Char *child, UInt8 mode)
{
    SLBootServicesCheck(kOSNullPointer);

    SLFile *file;
    SLStatus status = parent->open(parent, &file, child, mode, 0);
    bool failed = SLStatusIsError(status);

    return (failed ? kOSNullPointer : file);
}

SLFile *SLOpenPath(OSUTF8Char *path, UInt8 mode)
{
    SLBootServicesCheck(kOSNullPointer);

    OSUTF16Char *efiPath = SLPathToEFIPath(path);
    if (!efiPath) return kOSNullPointer;

    SLFile *root = SLGetRootDirectoryForImage(SLGetMainImageHandle());
    if (!root) return kOSNullPointer;

    SLFile *child = SLOpenChild(root, efiPath, mode);
    if (!child) return kOSNullPointer;

    return child;
}

bool SLCloseFile(SLFile *file)
{
    SLStatus status = file->close(file);
    return SLStatusIsError(status);
}

OSSize SLFileRead(SLFile *file, OSOffset offset, OSBuffer readBuffer)
{
    UInt64 currentOffset, size = readBuffer.size;
    SLStatus status = file->getOffset(file, &currentOffset);
    if (SLStatusIsError(status)) return false;

    if (currentOffset != (UInt64)offset)
    {
        status = file->setOffset(file, offset);
        if (SLStatusIsError(status)) return false;
    }
    
    status = file->read(file, &size, readBuffer.address);
    bool failed = SLStatusIsError(status);

    return (failed ? 0 : size);
}

OSBuffer SLFileReadFully(SLFile *file)
{
    OSUIDIntelData fileInfoUID = kSLFileInfoID;
    OSSize fileInfoSize = sizeof(SLFileInfo);
    SLFileInfo fileInfo;
    
    SLStatus status = file->getInfo(file, &fileInfoUID, &fileInfoSize, &fileInfo);
    if (status == kSLStatusBufferTooSmall) status = kSLStatusSuccess;
    if (SLStatusIsError(status)) return kOSBufferEmpty;
    
    OSBuffer buffer = SLAllocate(fileInfo.size);
    if (OSBufferIsEmpty(buffer)) return buffer;
    status = file->read(file, &buffer.size, buffer.address);

    if (SLStatusIsError(status))
    {
        buffer.size = fileInfo.size;
        SLFree(buffer.address);
        
        return kOSBufferEmpty;
    }
    
    return buffer;
}

bool SLReadPath(OSUTF8Char *path, OSOffset offset, OSBuffer readBuffer)
{
    SLFile *file = SLOpenPath(path, kSLFileModeRead);
    if (!file) return false;
    
    bool success = SLFileRead(file, offset, readBuffer);
    SLStatus status = file->close(file);
    
    if (SLStatusIsError(status))
        success = false;
    
    return success;
}

OSBuffer SLReadPathFully(OSUTF8Char *path)
{
    SLFile *file = SLOpenPath(path, kSLFileModeRead);
    if (!file) return kOSBufferEmpty;
    
    OSBuffer result = SLFileReadFully(path);
    SLStatus status = file->close(file);
    
    if (SLStatusIsError(status))
    {
        if (!OSBufferIsEmpty(result))
            SLFree(result.address);
        
        result = kOSBufferEmpty;
    }
    
    return result;
}

OSUTF16Char *SLPathToEFIPath(OSUTF8Char *path)
{
    return kOSNullPointer;
}
