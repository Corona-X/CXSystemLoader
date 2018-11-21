/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLKernelLoader.h - Kernel Loader main header                    */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane -  4.15.2017 -  2:30 PM PST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_KERNEL_SLKERNELLOADER__
#define __SYSTEMLOADER_KERNEL_SLKERNELLOADER__ 1

#include <Corona-X.h>
#include <System/OSTypes.h>
#include <SystemLoader/SLBase.h>
#include <SystemLoader/SLMach-O.h>

OSExport void CXKernelLoaderMain(SLMachOFile *loadedImage);

OSExport OSAddress gSLBootXAddress;

#endif /* !defined(__SYSTEMLOADER_KERNEL_SLKERNELLOADER__) */
