/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLBasicIO.h - Basic I/O for CXSystemLoader                      */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 8.8.2017   - 7:00 PM EST                           */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLBASICIO__
#define __SYSTEMLOADER_SLBASICIO__ 1

#include <Corona-X.h>
#include <System/OSTypes.h>
#include <System/OSCompilerMacros.h>

OSPrivate void SLPrintStringFromList(const OSUTF8Char *format, OSVAList args);

OSPrivate void SLPrintString(const OSUTF8Char *format, ...);

#endif /* !defined(__SYSTEMLOADER_SLBASICIO__) */
