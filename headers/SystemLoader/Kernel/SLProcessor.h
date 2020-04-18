/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLProcessor.h - CPU Routines for the KernelLoader.              */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane -  4.11.2020 - 10:00 PM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_KERNEL_SLPROCESSOR__
#define __SYSTEMLOADER_KERNEL_SLPROCESSOR__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

OSPrivate void SLProcessorEarlyInit(void);

OSPrivate void SLEarlyTrapHandler(void);

#endif /* !defined(__SYSTEMLOADER_KERNEL_SLPROCESSOR__) */
