#include <SystemLoader/Kernel/SLKernelLoader.h>
#include <SystemLoader/Kernel/SLEarlyMemoryInit.h>
#include <SystemLoader/Kernel/SLProcessorCheck.h>
#include <SystemLoader/Kernel/SLSerialConsole.h>
#include <SystemLoader/Kernel/SLCorePower.h>
#include <SystemLoader/Kernel/SLProcessor.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/Kernel/SLSMBIOS.h>
#include <SystemLoader/Kernel/SLVideo.h>
#include <SystemLoader/SLLibrary.h>
#include <SystemLoader/SLMach-O.h>
#include <SystemLoader/SLBase.h>

#include <Kernel/Shared/XKProcessorState.h>
#include <Kernel/Shared/XKProcessorOP.h>
#include <Kernel/Shared/XKLegacy.h>
#include <Kernel/C/CLUnicode.h>
#include <Kernel/C/CLMemory.h>

SLMemoryZoneInfo *gSLSystemZoneInfo;
SLMachOFile *gSLMachOFileSelf;

// Kill boot services
// Load dummy GDT, IDT, PML4
// Determine Memory Size
// Setup ACPI
// Map runtime services high
// Update runtime services map
// Setup IOAPIC/Multithreading
// Load Kernel + Servers
// Init Timing
// Init real VM
// Init logging
// Start other CPUs
// Trampoline through Kernel into SystemServer (Do this through iret after loading the boot CPU with its GPT)

// This function sets up the first page in memory.
// We copy '0xC1' to NULL, stamp our OS version right after,
// and then fill the rest of the page with zeros.
void SLSetupMem0(void)
{
    UInt8 mem0[0x1000];

    CLMemorySetValue(mem0, 0x1000, 0);
    mem0[0] = 0xC1;

    CLMemoryCopy(kCXLowMemoryString, &mem0[1], __builtin_strlen(kCXLowMemoryString));
    CLMemoryCopy(mem0, kOSNullPointer, 0x1000);
}

extern XKSegmentDescriptor _XKProcessorBootGDTR;

extern OSAddress _XKProcessorBootGDT;

void CXKernelLoaderMain(SLMachOFile *loadedImage)
{
    // This call is already made in this binary once control is passed here...
    //SLConsoleInitialize();

    SLDebugPrint("Entered CXKernelLoader!\n");
    SLDebugPrint("BootX.car loaded at %p\n", gSLBootXAddress);

    // 'loadedImage' is simply this binary
    gSLMachOFileSelf = loadedImage;

    // Yay make the screen pretty :)
    SLSetupVideo();

    OSAddress smbiosRootPointer = kOSNullPointer;
    OSAddress acpiRootPointer = kOSNullPointer;

    if (!SLProcessorValidate())
    {
        SLDebugPrint("Error: Incompatible CPU.\n");
        SLSerialConsoleReadKey(true);

        SLLeave(kSLStatusLoadError);
    }

    #if kCXBuildDev
        SLSystemTableDumpConfigTables(SLSystemTableGetCurrent());
    #endif /* kCXBuildDev */

    if (!(acpiRootPointer = SLSystemTableLocateConfigTable(kSLACPITableID)))
    {
        SLDebugPrint("Error: ACPI root table not found (Incompatible System/UEFI Implementation).\n");
        SLSerialConsoleReadKey(true);

        SLLeave(kSLStatusLoadError);
    }

    if (!(smbiosRootPointer = SLSystemTableLocateConfigTable(kSLSMBIOSTableID)))
    {
        SLDebugPrint("Warning: SMBIOS table not found. This may be fatal.\n");

        // We're going to continue in this case.
        // Maybe we have a weird virtual machine or something without SMBIOS.
        // As long as we can get the stuff we need from ACPI/the CPU,
        //   this shouldn't matter. It does indicate strongly that
        //   this is an incompatible system, however.
    }

    SLMemoryMap *map = SLBootServicesTerminate();

    // EFI Boot Services are now disabled. We've now committed to booting here...
    // This function installs our boot GPT, IDT, and page tables into the CPU
    SLProcessorEarlyInit();

    if (!SLDoEarlyMemoryInit(map))
    {
        SLDebugPrint("Early Memory Initialization Failed.\n");
        SLSerialConsoleReadKey(true);

        SLLeave(kSLStatusLoadError);
    }

    if (!SLCorePowerEarlyInit(acpiRootPointer))
    {
        SLDebugPrint("ACPI Initialization Failed.\n");
        SLSerialConsoleReadKey(true);

        SLLeave(kSLStatusLoadError);
    }

    if (smbiosRootPointer && !SMTableParse(smbiosRootPointer))
    {
        SLDebugPrint("SMBIOS Table parsing failed.\n");
        SLSerialConsoleReadKey(true);

        SLLeave(kSLStatusLoadError);
    }

    SLSerialConsoleReadKey(true);
    SLLeave(kSLStatusSuccess);
}

OSNoReturn void SLLeave(SLStatus status)
{
    if (gSLBootServicesEnabled)
        SLBootServicesExit(status);
    else
        SLRuntimeServicesResetSystem(kSLResetTypeShutdown, status, kOSNullPointer);

    // We shouldn't get here...
    hlt();
}

/*

 This is probably a PDE
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
 It's a PDE....

 Also, while this binary is still running, we need to setup entry 0 of PML4
 as a direct 1:1 map physical/virtual. When we jump to the kernel we can jump high.

 Given we have 48 bits for virtual, maybe we take highest ~2GB for the last 4 chunks.
 Then, MSB 11 (below 2^48 - 2^31 limit) gives firmware, 10 gives kernel, and 01/00 give
 server banks 1 and 0.

 Sooo, Servers are 0xFFFFFFFF80000000 --> 0xFFFFFFFFBFFFFFFF,
       Kernel   is 0xFFFFFFFFC0000000 --> 0xFFFFFFFFDFFFFFFF,
   and Firmware is 0xFFFFFFFFE0000000 --> 0xFFFFFFFFFFFFFFFF

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



/*

 Notes for stuff that will end up being in the kernel:

 So, one thing I want to try is basically have a single structure stuffed into one page which centralizes all of the state tracking for the whole kernel.
 Each CPU would likely end up caching it, so we'd have nice quick access to it all.

 From there, we could split things into cpu state, memory state, timing, etc. structures for each major subsystem in the kernel proper, each being pointed to from the main kernel state structure.



 Regarding CPU stuff in particular, we get some sane defaults setup here in the kernel loader for the boot CPU at least.

 When we load the kernel binary, we might want to append a few pages for some important structures and stuff for CPU states and whatnot.

 This would include copies for GDT, IDT, and TSS for each CPU, as well as interrupt stacks to fill out the TSS entries on each TSS.

 This should kill any risk of bus contention on any of these tables, and since we can get a CPU count from ACPI in KL, we can calculate exactly how much space to allocate.

 The pointers to all of these things we allocate should fill out per-cpu structures we create for each CPU, and in the kernel proper, %gs should be setup such that the base points to




 About the layout of our `real` GDT/IDT/TSS in kernel proper:

 We use a very simple GDT, since we are in long mode after all. (We can setup the final boot GDT and far return into the main entry of the kernel binary in memory, setting the new stack and GDT and such)

 Effectively, we only need a few segments in the GDT. Unfortunately, DPL acually does apply in long mode (at least according to the Intel spec, see vol. 3a, section 5.2.1),
    so we need separate code/data segments for user/kernel even in 64 bit mode.
 Here's what I beleive is a good layout:
 --> NULL Segment
 --> DPL 0 Data Segment
 --> DPL 0 Code Segment
 --> DPL 3 Data Segment
 --> DPL 3 Code Segment
 --> CPU-Specific TSS

 The LDT should simply be set to 0 (disabled). I may consider adding a DPL=2 code segment for semi-priveleged code, but that honestly doesn't seem useful since they sorta nuked segmentation in long mode....

 The IDT is pretty straightforward, 256 interrupt vectors, first 32 are exceptions/traps. Just make sure the right CPU-specific stacks are configured in the trap descriptors and everything should be fine.

 For the TSS, load up the proper IST/RSP values for each CPU, ltr and that's that.

 */


/*

 Application
 |
 - Task
    |
    - Thread

 Threads are schedulable. Let's start out using a similar algorithm to CFS in Linux.

 For VM management, I think we should see if we can do something similar to XNU but with more limits on over-allocation/overutilizing swap space

 That basically means compressing unused pages and storing them on disk (er idk we have to see about this...)


 Maybe we can put servers into a special priviledged module? Then whatever gets opened there becomes a server

 */
