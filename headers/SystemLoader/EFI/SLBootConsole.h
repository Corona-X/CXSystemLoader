/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLBootConsole.h - EFI Boot Console Function Declarations        */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 8.8.2017   - 11:15 PM EST                          */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_EFI_SLBOOTCONSOLE__
#define __SYSTEMLOADER_EFI_SLBOOTCONSOLE__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

// Note: This is part of CXSystemLoader ONLY
//   Do not attempt to use these functions in
//   CXKernelLoader, there will be a build error.

#if !kCXAssemblyCode

typedef OSEnum(UInt32) {
    kSLConsoleScreenModeText        = 0,
    kSLConsoleScreenModeGraphics    = 1
} SLConsoleScreenMode;

typedef struct {
    UInt16 scanCode;
    UInt16 keycode;
} SLKeyPress;

typedef struct {
    SLABI SLStatus (*reset)(OSAddress this, bool extendedVerify);
    SLABI SLStatus (*readKey)(OSAddress this, SLKeyPress *key);
    OSAddress waitForKey;
} SLConsoleInput;

typedef struct {
    SLABI SLStatus (*reset)(OSAddress this, bool extendedVerify);
    SLABI SLStatus (*output)(OSAddress this, const OSUTF16Char *string);
    SLABI SLStatus (*queryMode)(OSAddress this, UInt64 mode, UInt64 *columns, UInt64 *rows);
    SLABI SLStatus (*setMode)(OSAddress this, UInt64 mode);
    SLABI SLStatus (*setAttributes)(OSAddress this, UInt64 attributes);
    SLABI SLStatus (*clearScreen)(OSAddress this);
    OSAddress setCursorPosition;
    SLABI SLStatus (*enableCursor)(OSAddress this, bool enable);
    OSAddress currentMode;
} SLConsoleOutput;

// Yes, this protocol is optional.
// Yes, it's from EDK2
// No, I do not care. It's nice to have.
typedef struct {
    OSAddress getMode;
    SLABI SLStatus (*setMode)(OSAddress this, SLConsoleScreenMode mode);
    OSAddress lockInput;
} SLConsoleControl;

#if kCXBootloaderCode
    OSPrivate SLConsoleInput *SLBootConsoleGetInput(void);
    OSPrivate bool SLBootConsoleResetInput(void);
    OSPrivate UInt16 SLBootConsoleReadKey(bool shouldBlock);
    OSPrivate UInt64 SLBootConsoleReadNumber(UInt8 base, bool *success);

    OSPrivate SLConsoleOutput *SLBootConsoleGetOutput(void);
    OSPrivate bool SLBootConsoleResetOutput(void);
    OSPrivate bool SLBootConsoleOutput(OSUTF16Char *string);
    OSPrivate bool SLBootConsoleClearScreen(void);
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_EFI_SLBOOTCONSOLE__) */
