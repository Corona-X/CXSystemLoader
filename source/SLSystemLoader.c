#include <SystemLoader/SLSystemLoader.h>
#include <SystemLoader/SLBasicIO.h>
#include <SystemLoader/SLLibrary.h>
#include <Kernel/C/XKMemory.h>

OSPrivate UInt8 gSLInitialBlockBuffer[kSLBootPageSize];

UInt64 SLIsSystemPartition(SLBlockIO *blockDevice)
{
    if (blockDevice->media->blockSize > kSLBootPageSize)
        return 0;

    if (!SLBlockIOReadBlocks(blockDevice, 0, gSLInitialBlockBuffer, blockDevice->media->blockSize))
        return 0;

    if (XKMemoryCompare(gSLInitialBlockBuffer, kCAHeaderMagic, 4))
        return 0;

    if (XKMemoryCompare(&gSLInitialBlockBuffer[4], kCAHeaderVersionSystem, 4))
        return 0;

    UInt64 version;

    XKMemoryCopy(&gSLInitialBlockBuffer[8], &version, sizeof(CASystemVersionInternal));
    return version;
}

void SLPrintSystemVersionInfo(CASystemVersionInternal *version)
{
    switch (version->type)
    {
        case kCASystemTypeCoronaX: SLPrintString("Corona-X"); break;
        case kCASystemTypeCorOS:   SLPrintString("CorOS");    break;
        default: SLPrintString("???"); break;
    }
    
    UInt8 revision = version->revision + 'A';
    UInt32 majorVersion = version->majorVersion;
    SLPrintString(" version %c.%u (", revision, majorVersion);
    
    switch (version->buildType)
    {
        case kCASystemBuildTypeDebug:       SLPrintString("Debug");         break;
        case kCASystemBuildTypeDevelopment: SLPrintString("Development");   break;
        case kCASystemBuildTypeRelease:     SLPrintString("Release");       break;
        case kCASystemBuildTypeStable:      SLPrintString("Stable");        break;
        default:                            SLPrintString("?????");         break;
    }

    UInt64 buildID = version->buildID;
    SLPrintString("). Build ID 0x%012X", buildID);
}

void SLLoadSystemOrLeave(OSUnused SLBlockIO *blockDevice)
{
    SLPrintString("System loading doesn't work yet...\n");
    SLBootConsoleReadKey(true);

    SLLeave(kSLStatusLoadError);
}
