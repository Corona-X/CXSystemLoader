#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/EFI/SLBlockIO.h>
#include <SystemLoader/SLLibrary.h>

SLBlockIO **SLBlockIOGetAll(OSCount *count)
{
    return SLBootServicesLocateHandles(kSLBlockIOProtocol, count);
}

bool SLBlockIORead(SLBlockIO *object, OSOffset lba, OSAddress address, OSSize size)
{
    SLBootServicesCheck(false);

    SLStatus status = object->readBlocks(object, object->media->mediaID, lba, size, address);
    if (status == kSLStatusNoMedia) return false;

    return !SLStatusIsError(status);
}
