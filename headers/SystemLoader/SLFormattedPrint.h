/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLMemoryAllocator.h - Bootloader Memory Allocation Routines     */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 30.10.2016 - 3:45 AM EST                           */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 30.10.2016 - 3:45 AM EST                           */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLFORMATTEDPRINT__
#define __SYSTEMLOADER_SLFORMATTEDPRINT__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

#define kSLConsoleMaxNumber 8

typedef void (*SLConsoleOutput)(OSUTF8Char *string, OSSize size, OSAddress context);
typedef UInt8 (*SLConsoleInput)(bool wait, OSAddress context);
typedef void (*SLConsoleMoveBackward)(OSCount spaces, OSAddress context);
typedef void (*SLConsoleDeleteCharacters)(OSCount characters, OSAddress context);

typedef struct __SLConsole {
    UInt8 id;

    struct __SLConsole *next;
    OSAddress context;

    SLConsoleOutput output;
    SLConsoleInput input;

    UInt16 height;
    UInt16 width;

    SLConsoleMoveBackward moveBackward;
    SLConsoleDeleteCharacters deleteCharacters;
} SLConsole;

#if kCXBuildDev
    OSPrivate void SLMoveBackward(OSCount count);
    OSPrivate void SLDeleteCharacters(OSCount count);
    OSPrivate UInt8 *SLScanString(UInt8 terminator, OSSize *size);

    OSPrivate OSUTF8Char *SLUIDToString(SLProtocol *uid);
    OSPrivate OSUTF8Char *SLNumberToString(SInt64 number, bool isSigned, UInt8 base, UInt8 padding, UInt8 hexStart, OSLength *size);
    OSPrivate OSUTF8Char *SLPrintToString(const char *format, ...);
    OSPrivate void SLPrintString(const char *format, ...);
    OSPrivate OSUTF8Char *SLPrintToStringFromList(const char *format, OSVAList args);
    OSPrivate void SLPrintStringFromList(const char *f, OSVAList args);

    OSPrivate OSUTF8Char *SLUTF16ToUTF8(OSUTF16Char *utf16);
    OSPrivate OSUTF16Char *SLUTF8ToUTF16(OSUTF8Char *utf8);
    OSPrivate OSSize SLUTF16SizeInUTF8(OSUTF16Char *utf16);
    OSPrivate OSSize SLUTF8SizeInUTF16(OSUTF8Char *utf8);

    OSPrivate SInt8 SLRegisterConsole(SLConsole *console);
    OSExport SLConsole *gSLFirstConsole;
#endif /* kCXBuildDev */

#endif /* !defined(__SYSTEMLOADER_SLFORMATTEDPRINT__) */
