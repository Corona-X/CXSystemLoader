#include <SystemLoader/Kernel/SLKernelLoader.h>
#include <SystemLoader/Kernel/SLEarlyMemoryInit.h>
#include <SystemLoader/Kernel/SLProcessorCheck.h>
#include <SystemLoader/Kernel/SLSerialConsole.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/Kernel/SLVideo.h>
#include <SystemLoader/SLLibrary.h>
#include <SystemLoader/SLMach-O.h>
#include <SystemLoader/SLBase.h>

#include <Kernel/Shared/XKProcessorState.h>
#include <Kernel/Shared/XKProcessorOP.h>
#include <Kernel/Shared/XKLegacy.h>
#include <Kernel/C/XKUnicode.h>
#include <Kernel/C/XKMemory.h>

// Kill boot services
// Determine Memory Size
// Map Memory in CR3
// Setup IDT
// Read SMBIOS
// Setup ACPI
// Map runtime services high
// Update runtime services map
// Setup IOAPIC/Multithreading
// Load Kernel + Servers
// Init Timing
// Init real VM
// Init logging
// Start other CPUs
// Trampoline through Kernel into SystemServer

OSPrivate bool SLSetupMem0(void);

// This function sets up the first KiB in memory.
// We copy '0xC1' to NULL, stamp our OS version right after,
// and then fill the rest of the page with zeros.
bool SLSetupMem0(void)
{
    bool allocated = SLBootServicesAllocatePages(kOSNullPointer, 1);
    UInt8 mem0[0x400];

    if (!allocated)
        return false;

    XKMemorySetValue(mem0, 0x400, 0);
    mem0[0] = 0xC1;

    XKMemoryCopy(kCXLowMemoryString, &mem0[1], __builtin_strlen(kCXLowMemoryString));
    XKMemoryCopy(mem0, kOSNullPointer, 0x400);

    return true;
}

void CXKernelLoaderMain(OSUnused SLMachOFile *loadedImage)
{
    // This call is already made in this binary once control is passed here...
    //SLConsoleInitialize();

    if (kCXBuildDev)
    {
        SLPrintString("Entered CXKernelLoader!\n");
        SLPrintString("BootX.car loaded at %p\n", gSLBootXAddress);
    }

    // Set First KiB of RAM to System Info (Below legacy BIOS data area)
    if (!SLSetupMem0() && kCXBuildDev)
        SLPrintString("Warning: Null page allocated by firmware!\n");

    if (kCXBuildDev)
        SLPrintString("System info in first KiB: %s\n", (OSUTF8Char *)1);

    // Yay make the screen pretty :)
    SLSetupVideo();

    XKSegmentDescriptor gdtr, idtr;

    sgdt(&gdtr);
    sidt(&idtr);

    SLPrintString("GDT: %p\n", gdtr.base);
    SLPrintString("Length: 0x%04X\n", gdtr.limit);

    SLPrintString("IDT: %p\n", idtr.base);
    SLPrintString("Length: 0x%04X\n", idtr.limit);

    XKProcessorBasicState basicState;
    XKProcessorGetBasicState(&basicState);

    XKProcessorControlState controlState;
    XKProcessorGetControlState(&controlState);

    XKProcessorSegmentState segmentState;
    XKProcessorGetSegmentState(&segmentState);

    SLPrintString("CR0: %p\n", controlState.cr0);
    SLPrintString("CR3: %p\n", controlState.cr3);

    SLPrintString("RBP: %p\n", basicState.rbp);
    SLPrintString("RSP: %p\n", basicState.rsp);
    SLPrintString("RIP: %p\n", basicState.rip);

    SLPrintString("CS: 0x%04X\n", segmentState.cs);
    SLPrintString("GS: 0x%04X\n", segmentState.gs);

    SLMemoryMap *map = SLBootServicesTerminate();
    SLMemoryZoneInfo *zoneInfo = SLMemoryZoneRead(map);

    // Put as much RAM in as you want, we're only gonna map so much of it...
    if (zoneInfo->availableSize > ((1L << 48) - (1 << 31)))
        SLPrintString("Note: This system has a ton of memory.\n");

    // Why is our memory not a multiple of the sallest page size on this architecture?
    if (zoneInfo->availableSize % 0x1000)
    {
        // I'm not really sure what to do here...?
    }

    //

    SLProcessorValidate();

    SLSerialConsoleReadKey(true);
    SLLeave(kSLStatusSuccess);
}

OSNoReturn void SLLeave(SLStatus status)
{
    if (gSLBootServicesEnabled)
        SLBootServicesExit(status);
    else
        SLRuntimeServicesResetSystem(kSLResetTypeShutdown, status, kOSNullPointer);
}

/*

 +------------------+-------+
 |       ***        |   0   |
 +------------------+-------+
 |       ***        |   1   |
 +------------------+-------+
 |       ***        |   2   |
 +------------------+-------+
 |       ...        |  ...  |
 +------------------+-------+
 |  Server Code z2  | n - 4 |
 +------------------+-------+
 |  Server Code z1  | n - 3 |
 +------------------+-------+
 |    Kernel Code   | n - 2 |
 +------------------+-------+
 |  Firmware Space  | n - 1 |
 +------------------+-------+

 'n' is what exactly? Well let me tell you...
 'n' is the number of this level of page directory.

 Which level of page directory is this anyway?
 Another good question... I'm not actually sure yet...

 Also, while this binary is still running, we need to setup entry 0 of PML4
 as a direct 1:1 map physical/virtual. When we jump to the kernel we can jump high.

 Given we have 48 bits for virtual, maybe we take highest ~2GB for the last 4 chunks.
 Then, MSB 11 (below 2^48 - 2^31 limit) gives firmware, 10 gives kernel, and 01/00 give
 server banks 1 and 0.

 Sooo, Servers are 0x7FFFFFFF80000000 --> 0x7FFFFFFFBFFFFFFF,
       Kernel   is 0x7FFFFFFFC0000000 --> 0x7FFFFFFFDFFFFFFF,
   and Firmware is 0x7FFFFFFFE0000000 --> 0x7FFFFFFFFFFFFFFF

 Or, zero extending to 64 bits, 0xFFFF80000000 --> 0xFFFFFFFFFFFF are mapped virtually.

 Note: Firmware is ACPI/UEFI/whatever else we have that won't be used excessively.
       We can just sorta map addresses up to fit there, refill the boot service map,
          and tell the firmware where we put it. Then we can make direct calls natively.
 */

/*

Shared/
XKProcessorOP.h --> Define CPU instructions as inlines/macros
XKProcessorState.h --> Define CPU-related state structures
XKProcessorInfo.h --> Define static CPU feature information structures


 Note: For thread-specific information, we can use %gs to store a point to a CPU data struct in %gs. We can have both the volatile and non-volatile states from the state and info headers + current running program stuff in here.

 */

// Figure out how to do VM detection better???
