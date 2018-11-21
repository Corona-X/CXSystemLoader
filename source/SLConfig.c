#include <SystemLoader/Loader/SLConfig.h>
#include <SystemLoader/Loader/SLLoader.h>

#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/EFI/SLFile.h>
#include <SystemLoader/SLBasicIO.h>

#include <Kernel/C/XKMemory.h>

XKBootConfig *gSLCurrentConfig = kOSNullPointer;

XKBootConfig *SLConfigLoad(OSUTF8Char *path)
{
    if (gSLCurrentConfig) SLFree(gSLCurrentConfig);
    gSLCurrentConfig = SLAllocate(sizeof(XKBootConfig));

    gSLCurrentConfig->entries = kOSNullPointer;
    gSLCurrentConfig->entryCount = 0;

    OSUTF16Char *efiPath = SLPathToEFIPath(path);
    if (!efiPath) return gSLCurrentConfig;

    SLFile *file = SLOpenPath(efiPath, kSLFileModeRead);
    SLFree(efiPath);

    if (!file)
        return gSLCurrentConfig;

    OSSize fileSize;
    OSSize readSize;

    if (!SLFileGetSize(file, &fileSize))
        goto error;

    if (!fileSize || fileSize > kSLConfigMaxFileSize)
        goto error;

    OSUTF8Char *buffer = (OSUTF8Char *)SLFileReadFully(file, &readSize);

    if (readSize != fileSize)
    {
        if (buffer)
            SLFree(buffer);

        goto error;
    }

    if (!buffer)
        goto error;

    OSUTF8Char **buffers = kOSNullPointer;
    OSAddress hold = kOSNullPointer;
    OSLength maxNameLength = 0;
    OSIndex j = sizeof(OSSize);
    bool searchName = true;
    bool inComment = false;
    UInt64 entries = 0;
    OSCount count = 0;
    OSIndex entry = 0;
    UInt8 pass = 0;

    parse:

    for (OSIndex i = 0; i < (OSIndex)fileSize; i++)
    {
        OSUTF8Char c = buffer[i];

        if (inComment)
        {
            if (c != '\n') {
                continue;
            } else {
                inComment = false;
                continue;
            }
        }

        // '#' begins a comment
        if (c == '#')
        {
            inComment = true;
            continue;
        }

        // Ignore whitespace
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t')
            continue;

        switch (pass)
        {
            case 0: {
                if (c == '=' || c == ';') {
                    if (searchName && c == ';')
                    {
                        SLPrintString("Error: Malformed config file! (Found terminator when reading key)\n");
                        SLFree(buffer);

                        goto error;
                    }

                    if (!searchName && c == '=')
                    {
                        SLPrintString("Error: Malformed config file! (Found setter when reading value)\n");
                        SLFree(buffer);

                        goto error;
                    }

                    if (count > maxNameLength)
                        maxNameLength = count;

                    searchName = !searchName;
                    count = 0;
                    entries++;
                } else {
                    count++;
                }
            } break;
            case 1: {
                if (c == '=' || c == ';') {
                    OSSize *sizeEntry = (OSSize *)buffers[entry];
                    (*sizeEntry) = (j - sizeof(OSSize)) + 1;

                    // Add terminator
                    buffers[entry][j] = 0;

                    entry++;
                    j = sizeof(OSSize);
                } else {
                    buffers[entry][j++] = c;
                }
            } break;
        }
    }

    if ((++pass) < 2)
    {
        if (entries & 1)
        {
            // Why u do dis....
            SLPrintString("Error: Malformed config file!\n");
            SLFree(buffer);

            goto error;
        }

        if (!entries)
        {
            SLFree(buffer);
            goto error;
        }

        // Allocate buffers and stuff...
        buffers = SLAllocate(entries * sizeof(OSAddress));

        if (!buffers)
        {
            SLFree(buffer);
            goto error;
        }

        // This grabs space for an extra entry to keep the heap nicer on reallocation
        hold = SLAllocate(2 * (sizeof(OSSize) + maxNameLength));

        for (OSIndex i = 0; i < (OSIndex)entries; i++)
        {
            buffers[i] = SLAllocate(sizeof(OSSize) + maxNameLength);

            if (!buffers[i])
            {
                for (OSIndex k = i - 1; k >= 0; k--)
                    SLFree(buffers[k]);

                SLFree(buffers);
                SLFree(buffer);

                goto error;
            }
        }

        goto parse;
    }

    SLFree(buffer);

    // Now we have to unpack what we just did into the right sized buffers...
    gSLCurrentConfig->entryCount = entries >> 1;
    gSLCurrentConfig->entries = SLAllocate(entries * sizeof(OSAddress));

    // Free this so our first entry can be allocated here
    SLFree(hold);

    if (!gSLCurrentConfig->entries)
    {
        for (OSIndex i = 0; i < (OSIndex)entries; i++)
            SLFree(buffers[i]);

        SLFree(buffers);
        goto error;
    }

    for (OSIndex i = 0; i < (OSIndex)entries; i++)
    {
        OSSize size = *((OSSize *)buffers[i]);
        gSLCurrentConfig->entries[i] = SLAllocate(size);

        if (!gSLCurrentConfig->entries[i])
        {
            for (OSIndex k = i - 1; k >= 0; k--)
                SLFree(buffer);

            SLFree(buffers);
            goto error;
        }

        XKMemoryCopy(buffers[i] + sizeof(OSSize), gSLCurrentConfig->entries[i], size);
        SLFree(buffers[i]);
    }

    SLFree(buffers);

error:
    SLFileClose(file);
    return gSLCurrentConfig;
}

XKBootConfig *SLConfigGetCurrent(void)
{
    if (!gSLCurrentConfig)
        SLConfigLoad((OSUTF8Char *)kSLLoaderDataDirectory "\\" kSLLoaderConfigFile);

    return gSLCurrentConfig;
}

bool SLConfigGetBool(XKBootConfig *config, const OSUTF8Char *key, bool defaultValue)
{
    for (OSIndex i = 0; i < (OSIndex)(config->entryCount * 2); i += 2)
        if (!XKStringCompare8(key, config->entries[i]))
            return !XKStringCompare8((OSUTF8Char *)"yes", config->entries[i + 1]);

    return defaultValue;
}

#if kCXBuildDev

void SLConfigDump(XKBootConfig *config)
{
    SLPrintString("Config has %u entries:\n", config->entryCount);

    for (OSIndex i = 0; i < ((OSIndex)config->entryCount * 2); i += 2)
        SLPrintString("%u: %s --> %s\n", (i / 2), config->entries[i], config->entries[i + 1]);
}

#endif /* kCXBuildDev */
