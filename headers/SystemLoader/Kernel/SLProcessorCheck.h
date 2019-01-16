/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLProcessorCheck.h - Routines ensuring CX can run on this CPU   */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane -  4.15.2017 -  2:30 PM PST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_KERNEL_SLPROCESSORCHECK__
#define __SYSTEMLOADER_KERNEL_SLPROCESSORCHECK__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

// Validate that Corona-X supports the current CPU
// This ensures we have CPUID and checks the CPU features it exposes
// Maybe some other small checks, but since we're in long mode here,
//   we know more than if we were booting with legacy BIOS
OSPrivate bool SLProcessorValidate(void);

OSPrivate bool SLProcessorSupportsCPUID(void);

#endif /* !defined(__SYSTEMLOADER_KERNEL_SLPROCESSORCHECK__) */
