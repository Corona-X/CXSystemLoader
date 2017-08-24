#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLSystemLoader.h>
#include <SystemLoader/SLBasicIO.h>
#include <SystemLoader/SLLibrary.h>
#include <SystemLoader/SLLoader.h>
#include <SystemLoader/SLMach-O.h>
#include <System/OSByteMacros.h>
#include <Kernel/C/XKMemory.h>

#define kSLBlockSize      512

OSPrivate UInt8 gSLInitialBlockBuffer[kSLBootPageSize];

typedef struct {
    OSOffset offset;
    OSSize size;

    bool present;
} SLFileLocator;

OSPrivate SLFileLocator SLLocateFile(CAHeaderSystemImage *header, SLBlockIO *device, const OSUTF8Char *path);
OSPrivate CAHeaderBootX *SLLoadBootX(SLFileLocator file, SLBlockIO *device);
OSPrivate bool SLValidateBootX(CAHeaderBootX *header, OSSize size);
OSPrivate SLFileLocator SLLocateKernelLoader(CAHeaderBootX *header);
OSPrivate void SLRunKernelLoader(OSAddress base, SLFileLocator locator);

// TODO: At least validate the header...
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

SLFileLocator SLLocateFile(CAHeaderSystemImage *header, SLBlockIO *device, OSUnused const OSUTF8Char *path)
{
    OSOffset entryOffset = header->entryTableOffset - header->tocOffset;
    OSCount entryCount = (entryOffset - sizeof(UInt32)) / sizeof(OSOffset);
    OSOffset lba = header->tocOffset / kSLBlockSize;
    SLFileLocator file = { .present = false };

    // TODO: Handle large ToC/Entry Table
    OSOffset *toc = SLAllocate(header->dataSectionOffset - header->tocOffset);
    OSAddress entryTable = ((UInt8 *)toc) + entryOffset;
    if (!toc) return file;

    if (!SLBlockIOReadBlocks(device, lba, toc, SLGetObjectSize(toc)))
    {
        SLFree(toc);
        return file;
    }

    for (OSIndex i = 0; (UInt64)i < entryCount; i++)
    {
        UInt8 *entry = entryTable + toc[i];
        if (i && !toc[i]) break;

        #if kCXDebug
            switch (*entry)
            {
                case kCAEntryTypeDirectory: {
                    CASystemDirectoryEntry *realEntry = entry;
                    SLPrintString("D %s\n", realEntry->path);
                } break;
                case kCAEntryTypeFile: {
                    CASystemFileEntry *realEntry = entry;
                    SLPrintString("F %s\n", realEntry->path);
                } break;
            }
        #endif /* kCXDebug */

        if ((*entry) == kCAEntryTypeFile)
        {
            CASystemFileEntry *realEntry = entry;

            if (!XKStringCompare8(realEntry->path, path))
            {
                file.offset = realEntry->dataOffset;
                file.size = realEntry->dataSize;
                file.present = true;

                SLFree(toc);
                return file;
            }
        }
    }

    SLFree(toc);
    return file;
}

CAHeaderBootX *SLLoadBootX(SLFileLocator file, SLBlockIO *device)
{
    if (!file.present) return kOSNullPointer;

    OSOffset blockOffset = OSAlignDown(file.offset, kSLBlockSize);
    OSSize precedingBytes = blockOffset - file.offset;
    OSOffset lba = blockOffset / kSLBlockSize;

    OSSize blockSize = OSAlignUpward(file.size, kSLBlockSize);
    OSCount pageCount = (blockSize >> kSLBootPageShift) + 1;

    OSAddress buffer = SLBootServicesAllocateAnyPages(pageCount);

    if (!buffer)
    {
        SLPrintString("Could not allocate %zu pages for BootX!\n", pageCount);
        return kOSNullPointer;
    }

    // Align so that the first byte on the second page is the start of the archive header
    OSAddress readAddress = buffer + (kSLBootPageSize - precedingBytes);

    if (!SLBlockIOReadBlocks(device, lba, readAddress, blockSize))
    {
        SLBootServicesFreePages(buffer, pageCount);
        return kOSNullPointer;
    }

    return (CAHeaderBootX *)(buffer + kSLBootPageSize);
}

// TODO: Actually validate the archive
bool SLValidateBootX(CAHeaderBootX *header, OSUnused OSSize size)
{
    if (XKMemoryCompare(header->magic, kCAHeaderMagic, 4))
        return false;

    if (XKMemoryCompare(header->version, kCAHeaderVersionBootX, 4))
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
    file.offset = entry->dataOffset;
    file.size = entry->dataSize;
    file.present = true;

    return file;
}

void SLRunKernelLoader(OSAddress base, SLFileLocator locator)
{
    if (!locator.present) return;

    // TODO: Copy over instance data to kernel loader

    // TODO: Kernel loader arguments

    SLMachOExecute(base + locator.offset, locator.size);
}

void SLLoadSystemOrLeave(OSUnused SLBlockIO *blockDevice)
{
    if (blockDevice->media->blockSize < 512)
    {
        SLPrintString("Block Size is too small!\n");
        SLLeave(kSLStatusLoadError);
    }

    if (blockDevice->media->blockSize != 512 && blockDevice->media->blockSize != 4098)
    {
        SLPrintString("Bad Block Size!\n");
        SLLeave(kSLStatusLoadError);
    }

    CAHeaderSystemImage *header = SLAllocate(blockDevice->media->blockSize);
    if (!header) SLLeave(kSLStatusLoadError);

    if (!SLBlockIOReadBlocks(blockDevice, 0, header, blockDevice->media->blockSize))
        SLLeave(kSLStatusLoadError);

    SLFileLocator bootLocator = SLLocateFile(header, blockDevice, kSLLoaderBootArchivePath);
    bootLocator.offset += header->dataSectionOffset;

    if (bootLocator.present) {
        SLPrintString("Found '%s'. Data Offset: %zu, Size: %zu\n", kSLLoaderBootArchivePath, bootLocator.offset, bootLocator.size);
        CAHeaderBootX *bootX = SLLoadBootX(bootLocator, blockDevice);

        if (!bootX || !SLValidateBootX(bootX, bootLocator.size))
        {
            SLPrintString("Could not load BootX.car!\n");
            SLBootConsoleReadKey(true);

            SLLeave(kSLStatusLoadError);
        }

        SLPrintString("Loaded Boot-X Archive at address %p\n", bootX);
        SLFileLocator kernelLoader = SLLocateKernelLoader(bootX);

        if (!kernelLoader.present)
        {
            SLPrintString("Could not find Kernel Loader!\n");
            SLBootConsoleReadKey(true);

            SLLeave(kSLStatusLoadError);
        }

        SLRunKernelLoader(bootX, kernelLoader);
        SLPrintString("Could not run Kernel Loader!\n");
    } else {
        SLPrintString("Could not find '%s'!\n", kSLLoaderBootArchivePath);
    }

    SLBootConsoleReadKey(true);
    SLLeave(kSLStatusLoadError);
}
