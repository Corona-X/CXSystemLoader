#include <SystemLoader/Kernel/SLCorePower.h>
#include <SystemLoader/SLBasicIO.h>

OSPrivate bool CPRootDescriptorValidate(CPRootDescriptor *descriptor);
OSPrivate bool CPPowerTableValidate(CPSystemPowerTable *table);

OSPrivate CPSystemPowerTable *CPPowerTableLocate(CPRootTable *rootTable, UInt32 signature);
OSPrivate bool CPCheckFixedDescriptorTable(CPFixedDescriptorTable *fadt);

bool CPRootDescriptorValidate(CPRootDescriptor *descriptor)
{
    // Reference: Table 5-27, `Extended Checksum`, `Signature` fields, ACPI 6.3

    // This ACPI version is too old!
    if (descriptor->revision < kCPRootDescriptorVersionMin)
        return false;

    // The sum of the whole table has to add to 0
    UInt8 sum = 0;

    for (UInt8 i = 0; i < descriptor->length; i++)
        sum += ((UInt8 *)descriptor)[i];

    // We could also check the checksum of the first 20 bytes, but I assume it's okay if the whole table is okay...
    return !sum;
}

bool CPPowerTableValidate(CPSystemPowerTable *table)
{
    // Reference: Table 5-28, `checksum` field, ACPI 6.3
    // Just ensure the sum of the table is 0.
    UInt8 sum = 0;

    for (UInt32 i = 0; i < table->length; i++)
        sum += ((UInt8 *)table)[i];

    return !sum;
}

CPSystemPowerTable *CPPowerTableLocate(CPRootTable *rootTable, UInt32 signature)
{
    OSCount count = (rootTable->length - sizeof(CPRootTable)) / sizeof(OSAddress);
    CPSystemPowerTable **tables = ((OSAddress)rootTable) + sizeof(CPRootTable);

    for (OSIndex i = 0; i < (OSIndex)count; i++)
    {
        if (((CPSystemPowerTable *)tables[i])->signature == signature)
            return (CPSystemPowerTable *)tables[i];
    }

    return kOSNullPointer;
}

bool CPCheckFixedDescriptorTable(CPFixedDescriptorTable *fadt)
{
    if (fadt->flags & kCPFixedFlagHardwareReduced)
        return false;

    return true;
}

CPRootTable *SLCorePowerEarlyInit(OSAddress rootPointer)
{
    SLDebugPrint("Starting ACPI Initialization with root pointer: %p\n", rootPointer);
    SLDebugPrint("Signature of root pointer: '%08s'\n", &((CPRootDescriptor *)rootPointer)->signature);

    if (!CPRootDescriptorValidate(rootPointer))
        return kOSNullPointer;

    SLDebugPrint("Root Pointer successfully validated.\n");

    CPRootTable *rootTable = ((CPRootDescriptor *)rootPointer)->rootTablePointer;
    if (rootTable->signature != kCPSignatureRootTable) return false;
    if (rootTable->revision < kCPRootTableVersionMin)
    if (!CPPowerTableValidate(rootTable)) return false;
    // We have an XSDT now!!

    SLPrintString("Found ACPI Root Table '%04s' at %p (of length %d) from descriptor at %p...\n", rootTable, rootTable, rootTable->length, rootPointer);
    SLCorePowerDumpRootTables(rootTable);

    // I'd like to load things in ~ this order:
    // 1. FADT
    // 2. APIC (LAPIC)
    // 3. MCFG (PCI)
    // ...
    // n. DSDT
    // n. SSDT

    // Find the FADT (I believe this should usually be the first table if not always
    CPFixedDescriptorTable *fadt = CPPowerTableLocate(rootTable, kCPSignatureFixedDescriptorTable);

    SLDebugPrint("Found ACPI version %d.%d as per FADT at %p\n", fadt->revision, fadt->acpiMinorVersion, fadt);
    //if (fadt->revision < kCPF)
    SLDebugPrint("Hypervisor Identity: 0x%016X\n", fadt->hypervisorIdentity);

    SLDebugPrint("SCI Number: %d\n", fadt->sciNumber);
    SLDebugPrint("SMI Port: %d\n", fadt->smiPort);

    CPIntrCntrDescriptorTable *madt = CPPowerTableLocate(rootTable, KCPSignatureInterruptConfigTable);
    OSSize remaining = madt->length - sizeof(CPIntrCntrDescriptorTable);
    UInt8 *entry = (UInt8 *)&madt->interruptControllers;

    SLDebugPrint("CPU local interrupt controller at 0x%08X\n", madt->localIntrCntrAddress);
    SLDebugPrint("Interrupt Controllers:\n");

    while (remaining)
    {
        SLDebugPrint("0x%08X (0x%02X)\n", entry[0], entry[1]);

        if (entry[0] == 0) {
            CPLocalAPIC *apic = (CPLocalAPIC *)entry;

            SLDebugPrint("LAPIC for CPU ID 0x%02X, processor UID 0x%02X, flags 0x%08X.\n", apic->apicID, apic->processorUID, apic->flags);
        } else if (entry[0] == 1) {
            CPIOAPIC *apic = (CPIOAPIC *)entry;

            SLDebugPrint("I/O APIC at 0x%08X with global interrupt base at 0x%08X.\n", apic->address, apic->globalInterruptBase);
        } else if (entry[0] == 2) {
            CPIntrSourceOverride *interruptSource = (CPIntrSourceOverride *)entry;

            SLDebugPrint("Interrupt Source Override on bus 0x%02X. Interrupt source 0x%02X triggers global interrupt 0x%08X (flags 0x%04X)\n", interruptSource->bus, interruptSource->source, interruptSource->globalInterrupt, interruptSource->flags);
        } else {
            CPLocalAPICNMI *nmi = (CPLocalAPICNMI *)entry;

            SLDebugPrint("LAPIC NMI is connected to pin 0x%02X for CPU UID 0x%02X (flags 0x%04X).\n", nmi->nmiLint, nmi->processorUID, nmi->flags);
        }

        remaining -= entry[1];
        entry += entry[1];
    }

    return rootTable;
}

#if kCXBuildDev

void SLCorePowerDumpRootTables(CPRootTable *rootTable)
{
    OSCount count = (rootTable->length - sizeof(CPRootTable)) / sizeof(OSAddress);
    CPSystemPowerTable **tables = ((OSAddress)rootTable) + sizeof(CPRootTable);

    if (!count)
    {
        SLPrintString("No ACPI Descriptor Tables Found.\n");
        return;
    }

    SLPrintString("Found %u ACPI Descriptor Tables (from %p):\n", count, tables);

    for (OSIndex i = 0; i < (OSIndex)count; i++)
    {
        CPSystemPowerTable *table = tables[i];
        bool valid = CPPowerTableValidate(table);

        // Note: The first `table` pointer is cast to a string pointer. If we take &table->signature, the compiler warns us about soumething that won't actually happen
        SLPrintString("%02u: %04s at %p (%s)\n", i, table, table, (valid ? "valid" : "invalid"));
    }
}

#endif /* kCXBuildDev */
