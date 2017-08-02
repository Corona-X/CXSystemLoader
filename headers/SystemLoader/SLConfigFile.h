/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLConfigFile.h - Routines for Loading and Applying SL Config    */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 14.10.2016 - 6:00 PM EST                           */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLCONFIGFILE__
#define __SYSTEMLOADER_SLCONFIGFILE__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>
#include <Kernel/XKLegacy.h>

#if !kCXAssemblyCode

typedef OSEnum(UInt8, SLFileConsoleMode) {
    kSLFileConsoleModeCreateNew,
    kSLFileConsoleModeAppend,
};

typedef struct {
    struct __SLVideoConfig {
        UInt8  maxScreenCount;
        UInt32 maxScreenHeight;
        UInt32 maxScreenWidth;
        UInt32 foregroundColor;
        UInt32 backgroundColor;

        bool enabled;
    } videoConsole;
    struct __SLSerialConfig {
        XKSerialPort *ports;
        UInt8 portCount;

        UInt16 baudRate;
        UInt8 worldLength;
        UInt8 parityType;
        UInt8 stopBits;

        bool enableOutput;
        bool enableInput;
    } serialConsole;
    struct __SLLogConfig {
        SLFileConsoleMode mode;
        OSUTF8Char *filePath;
        OSSize size;
    } logConfig;

    OSUTF8Char *bootFile;
} SLConfigDev;

typedef struct __SLVideoConfig SLVideoConfig;
typedef struct __SLSerialConfig SLSerialConfig;
typedef struct __SLLogConfig SLLogConfig;

typedef struct {
    SLProtocol rootParitionID;
} SLConfig;

typedef struct {
    SLConfig config;
    SLConfigDev dev;

    OSUTF16Char *path;
} SLConfigFile;

#if kCXBootloaderCode
    OSPrivate SLConfigFile *SLConfigLoad(OSUTF8Char *path);
    OSPrivate SLConfigFile *SLConfigGet(void);
    OSPrivate bool SLConfigSave(SLConfigFile *config);
#endif /* kCXBootloaderCode */

#endif /* !kCXAssemblyCode */

#endif /* !defined(__SYSTEMLOADER_SLCONFIGFILE__) */
