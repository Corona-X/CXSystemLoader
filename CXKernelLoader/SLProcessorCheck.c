#include <SystemLoader/Kernel/SLProcessorCheck.h>
#include <Kernel/Shared/XKProcessorOP.h>
#include <SystemLoader/SLBasicIO.h>

bool SLProcessorValidate(void)
{
    // Note: I don't *think* there have been any 64-bit CPUS made that don't support cpuid...
    // As such, and given that we're already running in long mode, it should be fair to simply
    // call cpuid directly. If some exception occurs, we certainly don't support the given CPU.

    OSUTF8Char hypervisorName[12];
    OSUTF8Char cpuTypeName[12];
    OSUTF8Char cpuFullName[48];
    UInt32 eax, ebx, ecx, edx;
    UInt32 extendedMax;
    UInt32 max;

    cpuid(0);

    ((UInt32 *)cpuTypeName)[0] = ebx;
    ((UInt32 *)cpuTypeName)[1] = edx;
    ((UInt32 *)cpuTypeName)[2] = ecx;

    SLPrintString("CPUID (0): %012s\n", cpuTypeName);
    SLPrintString("Highest Supported: 0x%08X\n", eax);
    max = eax;

    cpuid(0x80000000);

    SLPrintString("Max Extended: 0x%08X\n", eax);
    extendedMax = eax;

    cpuid(1);

    SLPrintString("CPU Info:\n");
    SLPrintString("eax: 0x%08X, ebx: 0x%08X\n", eax, ebx);
    SLPrintString("ecx: 0x%08X, edx: 0x%08X\n", ecx, edx);

    bool hypervised = (ecx >> 31) ? true : false;
    cpuid(0x80000001);

    SLPrintString("Extended Info:\n");
    SLPrintString("eax: 0x%08X, ebx: 0x%08X\n", eax, ebx);
    SLPrintString("ecx: 0x%08X, edx: 0x%08X\n", ecx, edx);

    // By Intel specs, this will return 0x40000000 in eax if not hypervised.
    // (Or the leaf value for any CPU unimplemented leaf)
    cpuid(0x40000000);

    if (!hypervised && eax != 0x40000000)
        hypervised = true; // We got a value.

    if (hypervised) {
        // Running virtualized
        SLPrintString("Hypervised. VMM Info:\n");

        ((UInt32 *)hypervisorName)[0] = ebx;
        ((UInt32 *)hypervisorName)[1] = ecx;
        ((UInt32 *)hypervisorName)[2] = edx;

        SLPrintString("VMM Name: %012s\n", hypervisorName);
        SLPrintString("Max VM CPUID: 0x%08X\n", eax);
    } else {
        // Not running virtualized (or something is lying to me)
        SLPrintString("Not virtualized. (Probably)\n");
    }

    if (extendedMax < 0x80000008) {
        SLPrintString("Note: Assuming phys/virt address size.\n");
    } else {
        cpuid(0x80000008);

        SLPrintString("Physical Address Bits: %u\n", eax & 0xFF);
        SLPrintString("Virtual Address Bits: %u\n", eax >> 8);
    }

    cpuid(0x80000007);

    SLPrintString("edx: %p\n", edx);

    cpuid(0x40000010);

    SLPrintString("eax: %p\n", eax);

    cpuid(0x80000002);

    ((UInt32 *)cpuFullName)[0] = eax;
    ((UInt32 *)cpuFullName)[1] = ebx;
    ((UInt32 *)cpuFullName)[2] = ecx;
    ((UInt32 *)cpuFullName)[3] = edx;

    cpuid(0x80000003);

    ((UInt32 *)cpuFullName)[4] = eax;
    ((UInt32 *)cpuFullName)[5] = ebx;
    ((UInt32 *)cpuFullName)[6] = ecx;
    ((UInt32 *)cpuFullName)[7] = edx;

    cpuid(0x80000004);

    ((UInt32 *)cpuFullName)[ 8] = eax;
    ((UInt32 *)cpuFullName)[ 9] = ebx;
    ((UInt32 *)cpuFullName)[10] = ecx;
    ((UInt32 *)cpuFullName)[11] = edx;

    SLPrintString("CPU Full Name: %048s\n", cpuFullName);

    return true;
}
