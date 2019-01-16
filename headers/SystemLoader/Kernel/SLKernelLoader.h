/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLKernelLoader.h - Kernel Loader main header                    */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane -  4.15.2017 -  2:30 PM PST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_KERNEL_SLKERNELLOADER__
#define __SYSTEMLOADER_KERNEL_SLKERNELLOADER__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <SystemLoader/SLMach-O.h>

#define kCXLowMemoryString "Corona System " kCXSystemName " " kCXSystemRevision "." kCXSystemMajorVersion ""

#if !kCXAssemblyCode && kCXBootloaderCode

OSExport void CXKernelLoaderMain(SLMachOFile *loadedImage);

OSExport OSAddress gSLBootXAddress;

#endif /* !kCXAssemblyCode && kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_KERNEL_SLKERNELLOADER__) */
