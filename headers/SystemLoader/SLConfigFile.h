/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* SLConfigFile.h - Routines for Loading and Applying SL Config    */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 14.10.2016 - 6:00 PM EST                           */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/
/* beeselmane - 14.10.2016 - 6:00 PM EST                           */
/**=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=**/

#ifndef __SYSTEMLOADER_SLCONFIGFILE__
#define __SYSTEMLOADER_SLCONFIGFILE__ 1

#include <Corona-X.h>
#include <SystemLoader/SLBase.h>

typedef CXEnum(UInt8, SLFileConsoleMode) {
    kSLFileConsoleModeCreateNew,
    kSLFileConsoleModeAppend,
};

typedef struct {
    struct {
        UInt8 maxScreenCount;
        UInt32 maxScreenHeight;
        UInt32 maxScreenWidth;
        UInt32 backgroundColor;
        UInt32 foregroundColor;
        
        bool enabled;
    } videoConsole;
    struct {
        SLSerialPort *ports;
        UInt8 portCount;

        UInt16 baudRate;
        UInt8 worldLength;
        UInt8 parityType;
        UInt8 stopBits;

        bool enabled;
    } serialConsole;
    struct {
        OSSize size;
        UInt16 scrollAmount;
        OSUTF8Char *defaultBackingPath;
        
        bool enabled;
    } memoryConsole;
    struct {
        OSUTF8Char **paths;
        UInt32 pathCount;
        SLFileConsoleMode mode;
        
        bool enabled;
    } fileConsole;

    OSUTF8Char *bootFile;
} SLConfigDev;

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
#endif /* kCXBootloaderCode */

#endif /* !defined(__SYSTEMLOADER_SLCONFIGFILE__) */
