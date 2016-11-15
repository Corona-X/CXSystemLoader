#include <SystemLoader/SystemLoader.h>
#include <Kernel/CXKMemory.h>

SLConfigFile *gSLCurrentConfig = kOSNullPointer;;

SLConfigFile *SLConfigLoad(OSUTF8Char *path)
{
    if (gSLCurrentConfig) SLFree(gSLCurrentConfig);
    gSLCurrentConfig = SLAllocate(sizeof(SLConfigFile)).address;

    if (kCXBuildDev)
    {
        gSLCurrentConfig->dev.videoConsole.maxScreenCount = 255;
        gSLCurrentConfig->dev.videoConsole.maxScreenHeight = 0xFFFF;
        gSLCurrentConfig->dev.videoConsole.maxScreenWidth = 0xFFFF;
        gSLCurrentConfig->dev.videoConsole.backgroundColor = 0x000000;
        gSLCurrentConfig->dev.videoConsole.foregroundColor = 0xFFFFFF;
        gSLCurrentConfig->dev.videoConsole.enabled = true;

        gSLCurrentConfig->dev.serialConsole.ports = SLAllocate(sizeof(SLSerialPort)).address;
        gSLCurrentConfig->dev.serialConsole.ports[0] = 0x03F8;
        gSLCurrentConfig->dev.serialConsole.portCount = 1;
        gSLCurrentConfig->dev.serialConsole.baudRate = 57600;
        gSLCurrentConfig->dev.serialConsole.worldLength = kSLSerialWordLength8Bits;
        gSLCurrentConfig->dev.serialConsole.parityType = kSLSerialNoParity;
        gSLCurrentConfig->dev.serialConsole.stopBits = kSLSerial1StopBit;
        gSLCurrentConfig->dev.serialConsole.enabled = true;

        gSLCurrentConfig->dev.memoryConsole.size = (1 << 18);         // 258 KiB
        gSLCurrentConfig->dev.memoryConsole.scrollAmount = (1 << 10); // 1 KiB
        gSLCurrentConfig->dev.memoryConsole.defaultBackingPath = (OSUTF8Char *)kSLLoaderDataDirectory "/SLMemoryConsole.log";
        gSLCurrentConfig->dev.memoryConsole.enabled = false;

        gSLCurrentConfig->dev.fileConsole.paths = kOSNullPointer;
        gSLCurrentConfig->dev.fileConsole.pathCount = 0;
        gSLCurrentConfig->dev.fileConsole.mode = kSLFileConsoleModeCreateNew;
        gSLCurrentConfig->dev.fileConsole.enabled = false;

        gSLCurrentConfig->dev.bootFile = (OSUTF8Char *)"/boot.car";
    }

    gSLCurrentConfig->config.rootParitionID = ((OSUIDIntelData){0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}});

    gSLCurrentConfig->dev.videoConsole.maxScreenWidth = 1440;
    gSLCurrentConfig->dev.videoConsole.maxScreenHeight = 900;
    gSLCurrentConfig->dev.videoConsole.maxScreenCount = 1;

    SLFile *file = SLOpenPath(path);
    if (!file) return gSLCurrentConfig;

    SLCloseFile(file);
    return gSLCurrentConfig;
}

SLConfigFile *SLConfigGet(void)
{
    if (!gSLCurrentConfig)
        SLConfigLoad((OSUTF8Char *)kSLLoaderDataDirectory "/" kSLLoaderConfigFile);

    return gSLCurrentConfig;
}
