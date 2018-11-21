/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLSMBIOS.h - SMBIOS Routines for Kernel Loader (Uses prefix SM) */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane -  4.15.2017 -  2:30 PM PST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_KERNEL_SLSMBIOS__
#define __SYSTEMLOADER_KERNEL_SLSMBIOS__ 1

#include <Corona-X.h>
#include <System/OSTypes.h>
#include <System/OSCompilerMacros.h>

typedef struct {
    //
} SMEntryPoint;

OSPrivate bool SMTableParse(SMEntryPoint *entry);
OSPrivate SMEntryPoint *SMTableGetMain(void);

#endif /* !defined(__SYSTEMLOADER_KERNEL_SLDEBUG__) */
