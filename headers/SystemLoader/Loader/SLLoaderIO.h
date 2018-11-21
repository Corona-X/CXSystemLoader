/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLLoaderIO.h - SystemLoader I/O Implmentation                   */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 17.11.2018 -  3:15 PM TP                           */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_LOADER_SLLOADERIO__
#define __SYSTEMLOADER_LOADER_SLLOADERIO__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

#if !kCXAssemblyCode && kCXBootloaderCode

OSPrivate void SLConsoleInitialize(void);

OSPrivate void SLPrintCharacter(const OSUTF8Char character);
OSPrivate void SLPrintBufferFlush(void);

#endif /* kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_LOADER_SLLOADERIO__) */
