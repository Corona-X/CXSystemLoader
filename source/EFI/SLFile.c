#include <SystemLoader/EFI/SLFile.h>
#include <SystemLoader/EFI/SLLoadedImage.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLFormattedPrint.h>
#include <SystemLoader/SLLibrary.h>
#include <Kernel/XKMemory.h>

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
    if (!root) goto fail;

    SLFile *child = SLOpenChild(root, efiPath, mode);
    if (!child) goto fail;

    SLFree(efiPath);
    return child;

fail:
    SLFree(efiPath);

    return kOSNullPointer;
}

bool SLCloseFile(SLFile *file)
{
    SLBootServicesCheck(false);

    SLStatus status = file->close(file);
    bool failed = SLStatusIsError(status);

    return !failed;
}

OSSize SLFileRead(SLFile *file, OSOffset offset, OSBuffer readBuffer)
{
    SLBootServicesCheck(0);

    UInt64 size = readBuffer.size;
    UInt64 currentOffset;

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

bool SLReadPath(OSUTF8Char *path, OSOffset offset, OSBuffer readBuffer)
{
    SLBootServicesCheck(false);

    SLFile *file = SLOpenPath(path, kSLFileModeRead);
    if (!file) return false;

    bool success = SLFileRead(file, offset, readBuffer);
    if (!SLCloseFile(file)) success = false;

    return success;
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
}

OSUTF16Char *SLPathToEFIPath(OSUTF8Char *path)
{
    OSSize copySize = XKStringSize8(path) + 1;
    OSUTF8Char *copy = SLAllocate(copySize).address;
    XKMemoryCopy(path, copy, copySize);

    for (OSIndex i = 0; i < (copySize - 1); i++)
    {
        if (copy[i] == '/')
            copy[i] = '\\';
    }

    OSUTF16Char *efiPath = SLUTF8ToUTF16(copy);
    SLFree(copy);

    return efiPath;
}
