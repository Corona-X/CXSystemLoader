#include <SystemLoader/EFI/SLBlockIO.h>
#include <SystemLoader/SLLibrary.h>

SLBlockIO **SLBlockIOGetAll(OSCount *count)
{
    return SLBootServicesLocateHandles(kSLBlockIOProtocol, count);
}

bool SLBlockIOReadBlocks(SLBlockIO *object, OSOffset lba, OSAddress address, OSSize size)
{
    SLBootServicesCheck(false);

    SLStatus status = object->readBlocks(object, object->media->mediaID, lba, size, address);
    return !SLStatusIsError(status);
}
