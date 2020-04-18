/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLSMBIOS.h - SMBIOS Routines for Kernel Loader (Uses prefix SM) */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane -  4.15.2017 -  2:30 PM PST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_KERNEL_SLSMBIOS__
#define __SYSTEMLOADER_KERNEL_SLSMBIOS__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <System/OSCompilerMacros.h>

#if !kCXAssemblyCode && kCXBootloaderCode

#define kSMSignatureEntryPoint  {'_', 'S', 'M', '3', '_'}

typedef struct {
    UInt8 anchor_string[5];
    UInt8 checksum;
    UInt8 length;
    UInt8 smbiosMajorVersion;
    UInt8 smbiosMinorVersion;
    UInt8 smbiosDocumentRevision;
    UInt8 entryPointRevision;
    UInt8 reserved;
    UInt32 structureTableMaxSize;
    UInt64 structureTableAddress;
} SMEntryPoint;

OSPrivate bool SMTableParse(SMEntryPoint *entry);
OSPrivate SMEntryPoint *SMTableGetMain(void);

#endif /* !kCXAssemblyCode && kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_KERNEL_SLDEBUG__) */
