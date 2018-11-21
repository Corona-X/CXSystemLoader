#include <SystemLoader/EFI/SLBootServices.h>
#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLibrary.h>
#include <SystemLoader/SLBasicIO.h>
#include <SystemLoader/SLMach-O.h>
#include <Kernel/C/XKMemory.h>

SLMachOFile *SLMachOFileOpenMapped(OSAddress base, OSSize size)
{
    SLMachOFile *file = SLAllocate(sizeof(SLMachOFile));
    if (!file) return kOSNullPointer;

    file->dataSectionIndex = 0;
    file->symbolTableOffset = 0;
    file->symbolCount = 0;
    file->stringsOffset = 0;
    file->stringsSize = 0;

    file->stackAddress = kOSNullPointer;
    file->stackSize = 0;

    file->loadAddress = kOSNullPointer;
    file->loadedSize = 0;

    file->base = base;
    file->size = size;

    file->entryPoint = kOSNullPointer;
    file->header = file->base;

    if (file->header->magic != kOSMOMagic)
    {
        if (kCXBuildDev)
            SLPrintString("Mach-O Error: Invalid file magic.\n");

        SLFree(file);

        return kOSNullPointer;
    }

    if (file->header->machineType != kOSMOMachineTypeX86_64)
    {
        if (kCXBuildDev)
            SLPrintString("Mach-O Error: Invalid CPU type.\n");

        SLFree(file);
        return kOSNullPointer;
    }

    if (file->header->machineSubtype != kOSMOMachineSubtypeX86_64Any)
    {
        if (kCXBuildDev)
            SLPrintString("Mach-O Error: Invalid CPU subtype.\n");

        SLFree(file);
        return kOSNullPointer;
    }

    if (file->header->fileType != kOSMOFileTypeExecutable)
    {
        if (kCXBuildDev)
            SLPrintString("Mach-O Error: Invalid file type.\n");

        SLFree(file);
        return kOSNullPointer;
    }

    if (!(file->header->flags | kOSMOFlagPositionIndependant))
    {
        if (kCXBuildDev)
            SLPrintString("Mach-O Error: Invalid file flags.\n");

        SLFree(file);
        return kOSNullPointer;
    }

    OSSize loadCommandSize = sizeof(OSMOHeader) + file->header->loadCommandsSize;
    UInt64 loadedSize = 0;

    for (UInt64 pass = 0; pass < 2; pass++)
    {
        OSCount commandsLeft = file->header->loadCommandCount;
        OSOffset offset = sizeof(OSMOHeader);

        while (commandsLeft--)
        {
            if (offset + sizeof(OSMOLoadCommand) > loadCommandSize)
            {
                // Don't let it overflow
                if (kCXBuildDev)
                    SLPrintString("Mach-O Error: Invalid load commands.\n");

                SLFree(file);
                return kOSNullPointer;
            }

            OSMOLoadCommand *cmd = (OSMOLoadCommand *)(file->base + offset);

            if ((OSSize)(offset + cmd->size) > loadCommandSize)
            {
                // This overflows too
                if (kCXBuildDev)
                    SLPrintString("Mach-O Error: Invalid load command.\n");

                SLFree(file);
                return kOSNullPointer;
            }

            if (cmd->size < sizeof(OSMOLoadCommand))
            {
                // This will corrupt or cause an infinite loop
                if (kCXBuildDev)
                    SLPrintString("Mach-O Error: Overlapping load commands.\n");

                SLFree(file);
                return kOSNullPointer;
            }

            switch (cmd->command)
            {
                case kOSMOCommandSegment64: {
                    OSMOSegmentCommand *command = cmd;

                    if (cmd->size < sizeof(OSMOSegmentCommand))
                    {
                        if (kCXBuildDev)
                            SLPrintString("Mach-O Error: Command too small for type.\n");

                        SLFree(file);
                        return kOSNullPointer;
                    }

                    if (!pass) {
                        // Check validity and account size
                        OSSize sectionSize = command->size - sizeof(OSMOSegmentCommand);

                        if (sectionSize < (command->sectionCount * sizeof(OSMOSectionCommand)))
                        {
                            // Command is too small
                            if (kCXBuildDev)
                                SLPrintString("Mach-O Error: Sections don't fit in segment command.\n");

                            SLFree(file);
                            return kOSNullPointer;
                        }

                        if (command->virtualAddress & kSLBootPageMask)
                        {
                            // Segments should be page aligned in memory
                            if (kCXBuildDev)
                                SLPrintString("Mach-O Error: Segment virtual address not memory aligned.\n");

                            SLFree(file);
                            return kOSNullPointer;
                        }

                        if (command->offset + command->size > file->size)
                        {
                            // Command will take more data than we have to load
                            if (kCXBuildDev)
                                SLPrintString("Mach-O Error: Segment extends over file end.\n");

                            SLFree(file);
                            return kOSNullPointer;
                        }

                        file->loadedSize += command->virtualSize;
                    } else {
                        // Actually map the segment
                        if (kCXBuildDev)
                            SLPrintString("Mapping Segment %s (size %zu/%zu) %zu from %p\n", command->name, command->size, command->virtualSize, command->virtualAddress, file->loadAddress);

                        OSSize zeroSize = command->virtualSize - command->size;

                        if (kCXBuildDev)
                            SLPrintString("With %zu trailing empty bytes\n", zeroSize);

                        OSAddress destination = file->loadAddress + command->virtualAddress;
                        OSAddress source = file->base + command->offset;

                        XKMemoryCopy(source, destination, command->size);

                        if (zeroSize)
                        {
                            destination += command->size;
                            XKMemorySetValue(destination, 0, zeroSize);
                        }

                        loadedSize += command->virtualSize;
                    }
                } break;
                case kOSMOCommandUnixThread: {
                    OSMOThreadCommand *command = cmd;

                    if (cmd->size < sizeof(OSMOThreadCommand))
                    {
                        if (kCXBuildDev)
                            SLPrintString("Mach-O Error: Command too small for type.\n");

                        SLFree(file);
                        return kOSNullPointer;
                    }

                    OSSize stateSize = command->size - sizeof(OSMOThreadCommand);
                    OSMOThreadStateNative *state = command + 1;

                    if (command->type != kOSMOx86ThreadState64)
                        break;

                    if (!stateSize || stateSize < sizeof(OSMOThreadStateNative))
                    {
                        // Thread state is too small
                        if (kCXBuildDev)
                            SLPrintString("Mach-O Error: Thread state not large enough.\n");

                        SLFree(file);
                        return kOSNullPointer;
                    }

                    if (pass) {
                        if (state != file->entryPoint)
                        {
                            // Either there are two UnixThread commands or the memory changed...
                            // Just get out...
                            if (kCXBuildDev)
                                SLPrintString("Mach-O Error: Unixthread load error.\n");

                            if (file->loadAddress)
                                SLBootServicesFreePages(file->loadAddress, file->loadedSize >> kSLBootPageShift);

                            SLFree(file);
                            return kOSNullPointer;
                        }
                    } else {
                        file->entryPoint = state;
                    }
                } break;
                case kOSMOCommandSymbolTable: {
                    OSMOSymbolTableCommand *command = cmd;

                    if (cmd->size < sizeof(OSMOSymbolTableCommand))
                    {
                        if (kCXBuildDev)
                            SLPrintString("Mach-O Error: Command too small for type.\n");

                        SLFree(file);
                        return kOSNullPointer;
                    }

                    if (pass) {
                        bool isIdentical = true;

                        if (command->symbolTableOffset != file->symbolTableOffset)
                            isIdentical = false;

                        if (command->symbolCount != file->symbolCount)
                            isIdentical = false;

                        if (command->stringsOffset != file->stringsOffset)
                            isIdentical = false;

                        if (command->stringsSize != file->stringsSize)
                            isIdentical = false;

                        if (!isIdentical)
                        {
                            // Either there are two symbol table commands or the memory changed...
                            // I don't like either of those cases, get out.
                            if (kCXBuildDev)
                                SLPrintString("Mach-O Error: Symbol/Strings table load error.\n");

                            if (file->loadAddress)
                                SLBootServicesFreePages(file->loadAddress, file->loadedSize >> kSLBootPageShift);

                            SLFree(file);
                            return kOSNullPointer;
                        }
                    } else {
                        file->symbolTableOffset = command->symbolTableOffset;
                        file->symbolCount = command->symbolCount;

                        file->stringsOffset = command->stringsOffset;
                        file->stringsSize = command->stringsSize;
                    }
                } break;
                default: break;
            }

            offset += cmd->size;
        }

        if (pass) {
            // Make sure we loaded everything we needed to
            if (loadedSize != file->loadedSize)
            {
                if (kCXBuildDev)
                    SLPrintString("Mach-O Error: Loaded segment size not equal to predicted.\n");

                SLBootServicesFreePages(file->loadAddress, file->loadedSize >> kSLBootPageShift);
                SLFree(file);

                return kOSNullPointer;
            }
        } else {
            // Allocate space for segments and go back
            file->loadAddress = SLAllocateAnyPages(file->loadedSize >> kSLBootPageShift);

            if (!file->loadAddress)
            {
                if (kCXBuildDev)
                    SLPrintString("Mach-O Error: Couldn't allocate space for segments.\n");

                SLFree(file);
                return kOSNullPointer;
            }
        }
    }

    file->stackSize = kSLMachODefaultStackSize;
    file->stackAddress = SLAllocateAnyPages(file->stackSize >> kSLBootPageShift);

    if (!file->stackAddress)
    {
        // Dang, we almost made it...
        if (kCXBuildDev)
            SLPrintString("Mach-O Error: Couldn't allocate executable stack.\n");

        SLBootServicesFreePages(file->loadAddress, file->loadedSize >> kSLBootPageShift);
        SLFree(file);

        return kOSNullPointer;
    }

    // The stack grows downward on x86_64 :)
    file->stackAddress += file->stackSize;
    return file;
}

void SLMachOFileClose(SLMachOFile *file)
{
    SLBootServicesFreePages(file->stackAddress - file->stackSize, file->stackSize >> kSLBootPageShift);
    SLBootServicesFreePages(file->loadAddress, file->loadedSize >> kSLBootPageShift);

    SLFree(file);
}

// TOOD: Implement this
bool SLMachOValidate(SLMachOFile *file);

OSInteger SLMachOSetSymbolValues(SLMachOFile *file, const OSUTF8Char *const *symbols, OSCount count, const OSAddress *const *values, OSSize *symbolSizes)
{
    if (!file->symbolTableOffset || !file->symbolCount)
        return 0;

    if (!file->stringsOffset || !file->stringsSize)
        return 0;

    OSMOSymbolEntry *symbol = (OSMOSymbolEntry *)(file->base + file->symbolTableOffset);
    const OSUTF8Char *strings = file->base + file->stringsOffset;
    OSMOSymbolEntry *symbolTableEnd = symbol + file->symbolCount;
    OSCount replaced = 0;

    while (symbol < symbolTableEnd)
    {
        if (!(symbol->type & kOSMOSymbolFlagSymbolicDebug))
        {
            for (OSIndex i = 0; (OSCount)i < count; i++)
            {
                if (symbol->nameOffset > file->stringsSize)
                    break;

                if (!XKStringCompare8(symbols[i], strings + symbol->nameOffset))
                {
                    if (kCXBuildDev)
                    {
                        SLPrintString("Replacing symbol %s at %p ", strings + symbol->nameOffset, symbol->value);
                        SLPrintString("(%p, %p)\n", file->loadAddress + symbol->value, values[i]);
                    }

                    OSAddress destination = symbol->value + file->loadAddress;
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

bool SLMachOCallVoidFunction(SLMachOFile *file, const OSUTF8Char *name)
{
    if (!file->symbolTableOffset || !file->symbolCount)
        return 0;

    if (!file->stringsOffset || !file->stringsSize)
        return 0;

    OSMOSymbolEntry *symbol = (OSMOSymbolEntry *)(file->base + file->symbolTableOffset);
    const OSUTF8Char *strings = file->base + file->stringsOffset;
    OSMOSymbolEntry *symbolTableEnd = symbol + file->symbolCount;

    while (symbol < symbolTableEnd)
    {
        if (!(symbol->type & kOSMOSymbolFlagSymbolicDebug))
        {
           if (symbol->nameOffset > file->stringsSize)
            break;

            if (!XKStringCompare8(name, strings + symbol->nameOffset))
            {
                if (kCXBuildDev)
                    SLPrintString("Calling function '%s' at %p\n", strings + symbol->nameOffset, symbol->value);

                OSAddress entry = symbol->value + file->loadAddress;
                ((void (*)())entry)();

                return true;
            }
        }

        symbol++;
    }

    return false;
}

// I cheat here. I don't actually load the full thread state...
// I just jump to the entry point specified (offset by the right amount)
// Usually everything but the instruction pointer is null, however,
// so this should be a fine way to go about it...
OSNoReturn void SLMachOExecute(SLMachOFile *file)
{
    OSAddress entryPoint = file->entryPoint->rip + file->loadAddress;
    OSAddress stack = file->stackAddress;

    if (kCXBuildDev)
    {
        SLPrintString("Calling entry point at %p...\n", entryPoint);
        SLPrintString("Note: WILL switch stack to %p\n", stack);
    }

    asm __volatile__ ("movq %0, %%rsp ;"
                      "movq %0, %%rbp ;"
                      "jmp *%1" : :
                      "r" (stack),
                      "r" (entryPoint),
                      "D" (file)
                      );

    SLPrintString("Error: ????");
    SLLeave(kSLStatusLoadError);
}
