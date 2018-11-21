/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLSerialConsole.h - Kernel Loader Serial Console Interface      */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 20.11.2018 -  5:00 PM TP                           */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_KERNEL_SLSERIALCONSOLE__
#define __SYSTEMLOADER_KERNEL_SLSERIALCONSOLE__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <Kernel/Shared/XKLegacy.h>

#define kXKBootConfigKeySerialPort "Serial-Port"
#define kXKBootConfigKeySerialRate "Serial-Rate"

#if !kCXAssemblyCode && kCXBootloaderCode

typedef struct {
    XKSerialPort port;
    UInt32 rate;

    bool disabled;
} SLSerialConsole;

// This is exported so it can be called before the kernel loader's proper main function.
OSExport void SLConsoleInitialize(void);

// Note: This is blocking.
OSPrivate void SLPrintCharacter(const OSUTF8Char character);

OSPrivate OSUTF8Char SLSerialConsoleReadKey(bool shouldBlock);

OSExport SLSerialConsole gSLSerialConsole;

#endif /* kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_KERNEL_SLSERIALCONSOLE__) */
