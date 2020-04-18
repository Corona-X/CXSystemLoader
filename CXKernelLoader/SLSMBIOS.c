#include <SystemLoader/Kernel/SLSMBIOS.h>
#include <SystemLoader/SLBasicIO.h>

SMEntryPoint *SMTableGetMain(void)
{
    return kOSNullPointer;
}

bool SMTableParse(SMEntryPoint *entry)
{
    SLPrintString("SMBIOS Anchor String: %c%c%c%c%c\n", entry->anchor_string[0], entry->anchor_string[1], entry->anchor_string[2], entry->anchor_string[3], entry->anchor_string[4]);
    SLPrintString("SMBIOS Version: %d.%d.%d\n", entry->smbiosMajorVersion, entry->smbiosMinorVersion, entry->smbiosDocumentRevision);

    //

    return false;
}
