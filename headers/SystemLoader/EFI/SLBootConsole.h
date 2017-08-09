/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLBootConsole.h - EFI Boot Console Function Declarations        */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 8.8.2017   - 11:15 PM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_EFI_SLBOOTCONSOLE__
#define __SYSTEMLOADER_EFI_SLBOOTCONSOLE__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

#if !kCXAssemblyCode

typedef struct {
    UInt16 scanCode;
    UInt16 keycode;
} SLKeyPress;

typedef struct {
    SLABI SLStatus (*reset)(OSAddress this, bool extendedVerify);
    SLABI SLStatus (*readKey)(OSAddress this, SLKeyPress *key);
    OSAddress waitForKey;
} SLSimpleTextInput;

typedef struct {
    SLABI SLStatus (*reset)(OSAddress this, bool extendedVerify);
    SLABI SLStatus (*output)(OSAddress this, OSUTF16Char *string);
    SLABI SLStatus (*queryMode)(OSAddress this, UInt64 mode, UInt64 *columns, UInt64 *rows);
    SLABI SLStatus (*setMode)(OSAddress this, UInt64 mode);
    OSAddress setAttribute;
    SLABI SLStatus (*clearScreen)(OSAddress this);
    OSAddress setCursorPosition;
    SLABI SLStatus (*enableCursor)(OSAddress this, bool enable);
    OSAddress currentMode;
} SLSimpleTextOutput;

#if kCXBootloaderCode
    OSPrivate bool SLBootConsoleResetInput(void);
    OSPrivate UInt16 SLBootConsoleReadKey(bool shouldBlock);

    OSPrivate bool SLBootConsoleResetOutput(void);
    OSPrivate bool SLBootConsoleOutput(OSUTF16Char *string);
    OSPrivate bool SLBootConsoleGetMode(UInt64 mode, UInt64 *columns, UInt64 *rows);
    OSPrivate bool SLBootConsoleSetMode(UInt64 mode);
    OSPrivate bool SLBootConsoleClearScreen(void);
    OSPrivate bool SLBootConsoleEnableCursor(bool enable);
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_EFI_SLBOOTCONSOLE__) */
