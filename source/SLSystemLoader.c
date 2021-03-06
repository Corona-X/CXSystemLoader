#include <SystemLoader/Loader/SLSystemLoader.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/Loader/SLConfig.h>
#include <SystemLoader/Loader/SLLoader.h>
#include <SystemLoader/SLBasicIO.h>
#include <SystemLoader/SLLibrary.h>
#include <SystemLoader/SLMach-O.h>
#include <System/OSByteMacros.h>
#include <Kernel/C/CLMemory.h>

#define kSLSymbolCount      7
#define kSLBlockSize        512

typedef struct {
    OSUTF8Char *path;
    OSOffset offset;
    OSSize size;

    bool present;
} SLFileLocator;

OSPrivate SLFileLocator SLLocateBootX(CAHeaderSystemImage *header, SLBlockIO *device);
OSPrivate CAHeaderBootX *SLLoadBootX(SLFileLocator file, SLBlockIO *device);
OSPrivate bool SLValidateBootX(CAHeaderBootX *header, OSSize size);
OSPrivate SLFileLocator SLLocateKernelLoader(CAHeaderBootX *header);
OSPrivate OSNoReturn void SLRunKernelLoader(OSAddress base, OSSize size, SLFileLocator locator);

OSPrivate UInt8 gSLInitialBlockBuffer[kSLBlockSize];

// TODO: At least validate the header...
UInt64 SLIsSystemPartition(SLBlockIO *blockDevice)
{
    if (blockDevice->media->blockSize != kSLBlockSize)
        return 0;

    if (!SLBlockIORead(blockDevice, 0, gSLInitialBlockBuffer, kSLBlockSize))
        return 0;

    if (CLMemoryCompare(gSLInitialBlockBuffer, kCAHeaderMagic, 4))
        return 0;

    if (CLMemoryCompare(&gSLInitialBlockBuffer[4], kCAHeaderVersionSystem, 4))
        return 0;

    UInt64 version;
    CLMemoryCopy(&gSLInitialBlockBuffer[8], &version, sizeof(CASystemVersionInternal));

    return version;
}

void SLPrintSystemVersionInfo(CASystemVersionInternal *version)
{
    if (!version)
    {
        SLPrintString("<Error>\n");
        return;
    }

    UInt32 majorVersion = version->majorVersion;
    UInt8 revision = version->revision + 'A';
    UInt64 buildID = version->buildID;

    switch (version->type)
    {
        case kCASystemTypeCoronaX: SLPrintString("Corona-X"); break;
        case kCASystemTypeCorOS:   SLPrintString("CorOS");    break;
        default: SLPrintString("???"); break;
    }

    SLPrintString(" version %c.%u (", revision, majorVersion);

    switch (version->buildType)
    {
        case kCASystemBuildTypeDebug:       SLPrintString("Debug");         break;
        case kCASystemBuildTypeDevelopment: SLPrintString("Development");   break;
        case kCASystemBuildTypeRelease:     SLPrintString("Release");       break;
        case kCASystemBuildTypeStable:      SLPrintString("Stable");        break;
        default:                            SLPrintString("?????");         break;
    }

    SLPrintString("). Build ID 0x%012X", buildID);
}

SLFileLocator SLLocateBootX(CAHeaderSystemImage *header, SLBlockIO *device)
{
    OSAddress buffer = SLAllocate(kSLBlockSize);
    SLFileLocator file = { .present = false };

    // There's no boot archive here!
    if (!(~header->bootEntry) || !buffer)
        return file;

    OSOffset tocOffset = header->bootEntry * sizeof(OSOffset);
    OSOffset realOffset = header->tocOffset + tocOffset;
    OSOffset lba = OSAlignDown(realOffset, kSLBlockSize);
    OSOffset readOffset = realOffset - lba;

    if (!SLBlockIORead(device, lba / kSLBlockSize, buffer, kSLBlockSize))
    {
        SLFree(buffer);
        return file;
    }

    UInt64 entryOffset = *((UInt64 *)(buffer + readOffset));
    realOffset = header->entryTableOffset + entryOffset;
    lba = OSAlignDown(realOffset, kSLBlockSize);
    readOffset = realOffset - lba;

    if (!SLBlockIORead(device, lba / kSLBlockSize, buffer, kSLBlockSize))
    {
        SLFree(buffer);
        return file;
    }

    CASystemFileEntry *entry = buffer + readOffset;
    file.offset = header->dataSectionOffset + entry->dataOffset;
    file.size = entry->dataSize;
    file.path = entry->path;

    if (entry->type == kCAEntryTypeFile)
        file.present = true;

    SLFree(buffer);
    return file;
}

CAHeaderBootX *SLLoadBootX(SLFileLocator file, SLBlockIO *device)
{
    OSOffset blockOffset = OSAlignDown(file.offset, kSLBlockSize);
    OSSize precedingBytes = blockOffset - file.offset;
    OSOffset lba = blockOffset / kSLBlockSize;

    OSSize blockSize = OSAlignUpward(file.size, kSLBlockSize);
    OSCount pageCount = (blockSize >> kSLBootPageShift) + 1;

    if (blockSize & kSLBootPageMask)
        pageCount++;

    OSAddress buffer = SLBootServicesAllocateAnyPages(pageCount);

    if (!buffer)
    {
        if (kCXBuildDev)
            SLPrintString("Error: Could not allocate %zu pages for BootX!\n", pageCount);
        else
            SLPrintString("Error: Could not allocate memory!\n");

        return kOSNullPointer;
    }

    // Align so that the first byte on the second page is the start of the archive header
    OSAddress readAddress = buffer + (kSLBootPageSize - precedingBytes);

    if (!SLBlockIORead(device, lba, readAddress, blockSize))
    {
        SLBootServicesFreePages(buffer, pageCount);
        return kOSNullPointer;
    }

    // Return the second page where we put the header
    return (CAHeaderBootX *)(buffer + kSLBootPageSize);
}

// TODO: Actually validate the archive
bool SLValidateBootX(CAHeaderBootX *header, OSSize size)
{
    if (size <= sizeof(CAHeaderBootX))
        return false;

    if (CLMemoryCompare(header->magic, kCAHeaderMagic, 4))
        return false;

    if (CLMemoryCompare(header->version, kCAHeaderVersionBootX, 4))
        return false;

    return true;
}

SLFileLocator SLLocateKernelLoader(CAHeaderBootX *header)
{
    SLFileLocator file = { .present = false };

    if (!header->kernelLoaderEntry)
        return file;

    OSOffset tocOffset = sizeof(CAHeaderBootX) + sizeof(CADataModification);
    OSOffset *toc = (OSOffset *)(((OSAddress)header) + tocOffset);

    CAEntryS2 *entry = ((OSAddress)header) + header->entryTableOffset + toc[header->kernelLoaderEntry];
    file.offset = header->dataSectionOffset + entry->dataOffset;
    file.size = entry->dataSize;
    file.path = entry->path;

    if (entry->type == kCAEntryTypeFile)
        file.present = true;

    return file;
}

OSNoReturn void SLRunKernelLoader(OSAddress base, OSSize size, SLFileLocator locator)
{
    SLMachOFile *loader = SLMachOFileOpenMapped(base + locator.offset, locator.size);

    if (!loader)
    {
        SLPrintString("Error: Could not open Kernel Loader.\n");
        SLBootConsoleReadKey(true);

        SLLeave(kSLStatusLoadError);
    }

    // These get mangled somehow someway,
    // this is workaround...
    const OSUTF8Char *s0 = "_gSLLoaderSystemTable";
    const OSUTF8Char *s1 = "_gSLLoaderImageHandle";
    const OSUTF8Char *s2 = "_gXKBootConfig";
    const OSUTF8Char *s3 = "_gSLBootXAddress";
    const OSUTF8Char *s4 = "_gSLBootXSize";
    const OSUTF8Char *s5 = "_gSLMainPoolInfo";
    const OSUTF8Char *s6 = "_gSLCurrentHeap";

    const OSUTF8Char *const symbols[kSLSymbolCount] = {s0, s1, s2, s3, s4, s5, s6};

    OSAddress values[kSLSymbolCount] = {
        &gSLLoaderSystemTable,
        &gSLLoaderImageHandle,
        &gSLCurrentConfig,
        &base,
        &size
    };

    values[5] = SLAllocate(sizeof(SLMemoryPool));
    values[6] = SLAllocate(sizeof(SLHeap));

    if (!values[5] || !values[6])
    {
        SLPrintString("Error: Could not allocate memory.\n");
        SLBootConsoleReadKey(true);

        SLLeave(kSLStatusLoadError);
    }

    CLMemoryCopy(&gSLMainPoolInfo, values[5], sizeof(SLMemoryPool));
    CLMemoryCopy(&gSLCurrentHeap, values[6], sizeof(SLHeap));

    const OSSize sizes[kSLSymbolCount] = {
        sizeof(OSAddress),
        sizeof(OSAddress),
        sizeof(OSAddress),
        sizeof(OSAddress),
        sizeof(OSSize),
        sizeof(SLMemoryPool),
        sizeof(SLHeap)
    };

    OSCount replaced = SLMachOSetSymbolValues(loader, symbols, kSLSymbolCount, values, sizes);

    if (replaced != kSLSymbolCount)
    {
        SLPrintString("Error: Could not transfer state to Kernel Loader.\n");
        SLBootConsoleReadKey(true);

        SLLeave(kSLStatusLoadError);
    }

    bool called = SLMachOCallVoidFunction(loader, "_SLConsoleInitialize");

    if (!called)
    {
        SLPrintString("Error: Could not call into Kernel Loader.\n");
        SLBootConsoleReadKey(true);

        SLLeave(kSLStatusLoadError);
    }

    #if kCXBuildDev
        OSCount allocs = SLMemoryAllocatorGetPoolAllocCount();
        OSCount frees = SLMemoryAllocatorGetPoolFreeCount();

        SLPrintString("Memory allocations: %u\n", allocs);
        SLPrintString("Unfreed objects: %u\n", (allocs - frees));
    #endif /* kCXBuildDev */

    SLMachOExecute(loader);
}

void SLLoadSystemOrLeave(SLBlockIO *blockDevice)
{
    CAHeaderSystemImage *header = SLAllocate(kSLBlockSize);

    if (!header)
    {
        SLPrintString("Error: Couldn't allocate memory.\n");
        SLBootConsoleReadKey(true);

        SLLeave(kSLStatusLoadError);
    }

    if (!SLBlockIORead(blockDevice, 0, header, kSLBlockSize))
    {
        SLPrintString("Error: Couldn't read device.\n");
        SLBootConsoleReadKey(true);

        SLLeave(kSLStatusLoadError);
    }

    SLFileLocator bootLocator = SLLocateBootX(header, blockDevice);

    if (bootLocator.present) {
        SLDebugPrint("Found BootX at %s. Data Offset: %zu, Size: %zu\n", kSLLoaderBootArchivePath, bootLocator.offset, bootLocator.size);

        CAHeaderBootX *bootX = SLLoadBootX(bootLocator, blockDevice);

        if (!bootX)
        {
            SLPrintString("Error: Could not load BootX.car.\n");
            SLBootConsoleReadKey(true);

            SLLeave(kSLStatusLoadError);
        }

        if (!SLValidateBootX(bootX, bootLocator.size))
        {
            SLPrintString("Error: BootX is not a valid archive.\n");
            SLBootConsoleReadKey(true);

            SLLeave(kSLStatusLoadError);
        }

        SLDebugPrint("Boot-X Archive successfully loaded at address %p\n", bootX);

        SLFileLocator kernelLoader = SLLocateKernelLoader(bootX);

        if (!kernelLoader.present)
        {
            SLPrintString("Error: Could not locate Kernel Loader.\n");
            SLBootConsoleReadKey(true);

            SLLeave(kSLStatusLoadError);
        }

        SLDebugPrint("Kernel Loader %zu from start of BootX at %p\n", kernelLoader.offset, ((OSAddress)bootX) + kernelLoader.offset);

        SLPrintString("Running Kernel Loader...\n");
        SLRunKernelLoader(bootX, bootLocator.size, kernelLoader);
    } else {
        SLPrintString("Could not locate BootX.\n");
        SLBootConsoleReadKey(true);

        SLLeave(kSLStatusLoadError);
    }
}
