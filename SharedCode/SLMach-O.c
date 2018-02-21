#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLBasicIO.h>
#include <SystemLoader/SLMach-O.h>
#include <Kernel/C/XKMemory.h>

OSPrivate bool SLMachOActivate(SLMachOFile *file);

bool SLMachOActivate(SLMachOFile *file)
{
    SLPrintString("Allocating %zu pages for Kernel Loader in memory\n", (file->virtualSize >> kSLBootPageShift));
    file->activateBase = SLBootServicesAllocateAnyPages(file->virtualSize >> kSLBootPageShift);
    if (!file->activateBase) return false;

    OSAddress address = file->base + sizeof(OSMOHeader);
    OSAddress end = address + file->header->loadCommandSize;

    while (address < end)
    {
        OSMOLoadCommand *cmd = address;

        if (cmd->command == kOSMOCommandSegment64)
        {
            OSMOSegmentCommand *command = cmd;
            OSAddress destination = file->activateBase + command->virtualAddress;
            OSAddress source = file->base + command->offset;

            if (kCXDebug)
            {
                SLPrintString("Segment '%s' wants %zu bytes at %p\n", command->name, command->virtualSize, command->virtualAddress);
                SLPrintString("Copying %zu bytes from %p to %p\n", command->size, source, destination);
            }

            XKMemoryCopy(source, destination, command->size);
        }

        address += cmd->size;
    }

    return true;
}

SLMachOFile *SLMachOProcess(OSAddress base, OSSize size)
{
    SLMachOFile *file = SLAllocate(sizeof(SLMachOFile));
    if (!file) return kOSNullPointer;

    file->base = base;
    file->size = size;

    file->header = file->base;
    file->entryPoint = kOSNullPointer;
    file->sectionCount = 0;

    if (file->header->magic != kOSMOMagic)
    {
        SLPrintString("Incorrect magic number '0x%08X'!\n", file->header->magic);
        SLFree(file);

        return kOSNullPointer;
    }

    if (file->header->fileType != kOSMOFileTypeExecutable)
    {
        SLPrintString("File is not an executable (0x%02X)!\n", file->header->fileType);
        SLFree(file);

        return kOSNullPointer;
    }

    OSAddress address = file->base + sizeof(OSMOHeader);
    OSAddress end = address + file->header->loadCommandSize;

    if (kCXDebug)
        SLPrintString("Load commands (%u):\n", file->header->loadCommandCount);

    while (address < end)
    {
        OSMOLoadCommand *command = address;

        switch (command->command)
        {
            case kOSMOCommandSegment64: {
                OSMOSegmentCommand *segmentCommand = command;
                file->sectionCount += segmentCommand->sectionCount;
                file->virtualSize += segmentCommand->virtualSize;

                if (kCXDebug)
                    SLPrintString("%zu (Segment, %u sectioons)\n", command->command, segmentCommand->sectionCount);
            } break;
            case kOSMOCommandSymbolTable: {
                OSMOSymbolTableCommand *symbolCommand = command;

                file->symbolOffset = symbolCommand->symbolTableOffset;
                file->symbolCount = symbolCommand->symbolCount;
                file->stringOffset = symbolCommand->stringTableOffset;

                if (kCXDebug)
                    SLPrintString("%zu (Symbol Table, %u symbols, %u strings)\n", command->command, file->symbolCount, symbolCommand->stringCount);
            } break;
            case kOSMOCommandUnixThread: {
                OSMOThreadCommand *threadCommand = command;
                OSMOThreadStateNative *threadState = threadCommand + 1;

                if (kCXDebug)
                    SLPrintString("%zu (Unixthread, rip=%p)\n", command->command, threadState->rip);

                if (file->entryPoint) {
                    SLFree(file);
                    return kOSNullPointer;
                } else {
                    // Remember this place...
                    if (threadCommand->type == kOSMOx86ThreadState64)
                        file->entryPoint = threadState;
                }
            } break;
            default: if (kCXDebug) {
                SLPrintString("%zu\n", command->command);
            } break;
        }

        address += command->size;
    }

    if (!file->entryPoint)
    {
        SLPrintString("No supported entry point!\n");
        SLFree(file);

        return kOSNullPointer;
    }

    if (!SLMachOActivate(file))
    {
        SLPrintString("Could not activate file!\n");
        SLFree(file);

        return kOSNullPointer;
    }

    return file;
}

void SLMachOClose(SLMachOFile *file)
{
    SLBootServicesFree(file->activateBase);
    SLFree(file);
}

// TOOD: Implement this
bool SLMachOValidate(SLMachOFile *file);

OSInteger SLMachOReplaceSymbols(SLMachOFile *file, const OSUTF8Char *const *symbols, OSCount count, const OSAddress *const *values, OSSize *symbolSizes)
{
    const OSUTF8Char *stringTable = file->activateBase + file->stringOffset;
    OSMOSymbolEntry *symbol = (OSMOSymbolEntry *)(file->activateBase + file->symbolOffset);
    OSAddress end = symbol + file->symbolCount;

    OSCount replaced = 0;

    while (((OSAddress)symbol) < end)
    {
        if (!(symbol->type & kOSMOSymbolFlagSymbolicDebug))
        {
            for (OSIndex i = 0; i < (OSIndex)count; i++)
            {
                if (!XKStringCompare8(symbols[i], stringTable + symbol->nameOffset))
                {
                    SLPrintString("Replacing symbol '%s'\n", symbols[i]);
                    OSAddress destination = symbol->value + file->activateBase;
                    XKMemoryCopy(values[i], destination, symbolSizes[i]);
    
                    replaced++;
                    break;
                }
            }
        }

        symbol++;
    }

    return replaced;
}

// I cheat here. I don't actually load the full thread state...
// I just jump to the entry point specified (offset by the right amount)
// Usually everything but the instruction pointer is null, however,
// so this should be a fine way to go about it...
void SLMachOExecute(SLMachOFile *file)
{
    OSAddress entryPoint = file->entryPoint->rip + file->activateBase;

    if (kCXDebug)
    {
        SLPrintString("Calling entry point at %p...\n", entryPoint);
        SLPrintString("First program byte is 0x%02X\n", *((UInt8 *)entryPoint));
    }

    ((void (*)(void))entryPoint)();

    return;
}
