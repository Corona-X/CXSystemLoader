#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLConfigFile.h>
#include <SystemLoader/EFI/SLFile.h>
#include <SystemLoader/SLBasicIO.h>
#include <SystemLoader/SLLoader.h>

SLConfigFile *gSLCurrentConfig = kOSNullPointer;

SLConfigFile *SLConfigLoad(OSUTF8Char *path)
{
    if (gSLCurrentConfig) SLFree(gSLCurrentConfig);
    gSLCurrentConfig = SLAllocate(sizeof(SLConfigFile));

    gSLCurrentConfig->rootParitionID = ((OSUIDIntelData){0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}});

    SLFile *file = SLOpenPath(path, kSLFileModeRead);
    if (!file) return gSLCurrentConfig;

    //

    SLFileClose(file);
    return gSLCurrentConfig;
}

SLConfigFile *SLConfigGet(void)
{
    if (!gSLCurrentConfig)
        SLConfigLoad((OSUTF8Char *)kSLLoaderDataDirectory "/" kSLLoaderConfigFile);

    return gSLCurrentConfig;
}

bool SLConfigSave(OSUnused SLConfigFile *config)
{
    SLPrintString("%s is not yet implemented!\n", __func__);
    return false;
}
