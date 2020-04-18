#include <SystemLoader/SLMemoryAllocator.h>
#include <SystemLoader/SLLibrary.h>
#include <SystemLoader/SLBasicIO.h>
#include <SystemLoader/SLMach-O.h>
#include <Kernel/C/CLMemory.h>

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
    file->stackSize = kSLMachODefaultStackSize;

    file->minLoadAddress = 0xFFFFFFFFFFFFFFFF;
    file->maxLoadAddress = kOSNullPointer;

    file->loadAddress = kOSNullPointer;
    file->loadedSize = 0;

    file->base = base;
    file->size = size;

    file->entryPoint = kOSNullPointer;
    file->header = file->base;

    if (file->header->magic != kOSMOMagic)
    {
        SLDebugPrint("Mach-O Error: Invalid file magic.\n");
        SLFree(file);

        return kOSNullPointer;
    }

    if (file->header->machineType != kOSMOMachineTypeX86_64)
    {
        SLDebugPrint("Mach-O Error: Invalid CPU type.\n");
        SLFree(file);

        return kOSNullPointer;
    }

    if (file->header->machineSubtype != kOSMOMachineSubtypeX86_64Any)
    {
        SLDebugPrint("Mach-O Error: Invalid CPU subtype.\n");
        SLFree(file);

        return kOSNullPointer;
    }

    if (file->header->fileType != kOSMOFileTypeExecutable)
    {
        SLDebugPrint("Mach-O Error: Invalid file type.\n");
        SLFree(file);

        return kOSNullPointer;
    }

    if (!(file->header->flags | kOSMOFlagPositionIndependant))
    {
        SLDebugPrint("Mach-O Error: Invalid file flags.\n");
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
                SLDebugPrint("Mach-O Error: Invalid load commands.\n");
                SLFree(file);

                return kOSNullPointer;
            }

            OSMOLoadCommand *cmd = (OSMOLoadCommand *)(file->base + offset);

            if ((OSSize)(offset + cmd->size) > loadCommandSize)
            {
                // This overflows too
                SLDebugPrint("Mach-O Error: Invalid load command.\n");
                SLFree(file);

                return kOSNullPointer;
            }

            if (cmd->size < sizeof(OSMOLoadCommand))
            {
                // This will corrupt or cause an infinite loop
                SLDebugPrint("Mach-O Error: Load command of size '%u' too small! (Note: %u commands remaining)\n", cmd->size, commandsLeft);
                SLFree(file);

                return kOSNullPointer;
            }

            switch (cmd->command)
            {
                case kOSMOCommandSegment64: {
                    OSMOSegmentCommand *command = cmd;

                    if (cmd->size < sizeof(OSMOSegmentCommand))
                    {
                        SLDebugPrint("Mach-O Error: Command too small for type.\n");
                        SLFree(file);

                        return kOSNullPointer;
                    }

                    if (!pass) {
                        // Check validity and account size
                        OSSize sectionSize = command->size - sizeof(OSMOSegmentCommand);

                        if (sectionSize < (command->sectionCount * sizeof(OSMOSectionCommand)))
                        {
                            // Command is too small
                            SLDebugPrint("Mach-O Error: Sections don't fit in segment command.\n");
                            SLFree(file);

                            return kOSNullPointer;
                        }

                        if (command->virtualAddress & kSLBootPageMask)
                        {
                            // Segments should be page aligned in memory
                            SLDebugPrint("Mach-O Error: Segment virtual address not memory aligned.\n");
                            SLFree(file);

                            return kOSNullPointer;
                        }

                        if (command->offset + command->size > file->size)
                        {
                            // Command will take more data than we have to load
                            SLDebugPrint("Mach-O Error: Segment extends over file end.\n");
                            SLFree(file);

                            return kOSNullPointer;
                        }

                        if (command->virtualAddress < file->minLoadAddress)
                            file->minLoadAddress = command->virtualAddress;

                        if ((command->virtualAddress + command->virtualSize) > file->maxLoadAddress)
                            file->maxLoadAddress = command->virtualAddress + command->virtualSize;

                        file->loadedSize += command->virtualSize;
                    } else {
                        // Actually map the segment
                        SLDebugPrint("Mapping Segment %s (size %zu/%zu) @ %zu bytes from %p\n", command->name, command->size, command->virtualSize, command->virtualAddress, file->loadAddress);

                        OSSize zeroSize = command->virtualSize - command->size;

                        SLDebugPrint("With %zu trailing bytes\n", zeroSize);

                        OSAddress destination = file->loadAddress + command->virtualAddress;
                        OSAddress source = file->base + command->offset;

                        CLMemoryCopy(source, destination, command->size);

                        if (zeroSize)
                        {
                            destination += command->size;
                            CLMemorySetValue(destination, 0, zeroSize);
                        }

                        loadedSize += command->virtualSize;
                    }
                } break;
                case kOSMOCommandUnixThread: {
                    OSMOThreadCommand *command = cmd;

                    if (cmd->size < sizeof(OSMOThreadCommand))
                    {
                        SLDebugPrint("Mach-O Error: Command too small for type.\n");
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
                        SLDebugPrint("Mach-O Error: Thread state not large enough.\n");
                        SLFree(file);

                        return kOSNullPointer;
                    }

                    if (pass) {
                        if (state != file->entryPoint)
                        {
                            // Either there are two UnixThread commands or the memory changed...
                            // Just get out...
                            SLDebugPrint("Mach-O Error: Unixthread load error.\n");

                            if (file->loadAddress)
                                SLFreePages(file->stackAddress - file->stackSize, (file->loadedSize + file->stackSize) >> kSLBootPageShift);

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
                        SLDebugPrint("Mach-O Error: Command too small for type.\n");
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
                            SLDebugPrint("Mach-O Error: Symbol/Strings table load error.\n");

                            if (file->loadAddress)
                                SLFreePages(file->stackAddress - file->stackSize, (file->loadedSize + file->stackSize) >> kSLBootPageShift);

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
            // Make sure we didn't do too much
            if (loadedSize > file->loadedSize)
            {
                SLDebugPrint("Mach-O Error: Loaded segment size greater than predicted.\n");

                SLFreePages(file->stackAddress - file->stackSize, (file->loadedSize + file->stackSize) >> kSLBootPageShift);
                SLFree(file);

                return kOSNullPointer;
            }
        } else {
            if ((file->maxLoadAddress - file->minLoadAddress) > file->loadedSize)
            {
                SLDebugPrint("Note: Adjusting load size by %d bytes.\n", (file->maxLoadAddress - file->minLoadAddress) - file->loadedSize);

                file->loadedSize = file->maxLoadAddress - file->minLoadAddress;
            }

            SLDebugPrint("Allocating %lu + %lu bytes for Mach-O file + stack.\n", file->loadedSize, file->stackSize);

            // Allocate space for segments and go back
            file->loadAddress = SLAllocatePages((file->loadedSize + file->stackSize) >> kSLBootPageShift);

            // Note: We put the stack right before the binary here.
            //       the stack grows down on x86_64, so if the stack overflows,
            //       it won't run into the end of the binary itself.
            file->loadAddress += file->stackSize; // Binary located just past the start of the stack
            file->stackAddress = file->loadAddress;

            if (!file->loadAddress)
            {
                SLDebugPrint("Mach-O Error: Couldn't allocate space for segments (+stack).\n");
                SLFree(file);

                return kOSNullPointer;
            }
        }
    }

    return file;
}

void SLMachOFileClose(SLMachOFile *file)
{
    SLFreePages(file->stackAddress - file->stackSize, (file->loadedSize + file->stackSize) >> kSLBootPageShift);

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

                if (!CLStringCompare8(symbols[i], strings + symbol->nameOffset))
                {
                    SLDebugPrint("Replacing symbol %s at %p ", strings + symbol->nameOffset, symbol->value);
                    SLDebugPrint("(%p, %p)\n", file->loadAddress + symbol->value, values[i]);

                    OSAddress destination = symbol->value + file->loadAddress;
                    CLMemoryCopy(values[i], destination, symbolSizes[i]);

                    replaced++;
                    break;
                }
            }
        }

        symbol++;
    }

    return replaced;
}

OSAddress SLMachOGetSymbolAddress(SLMachOFile *file, const OSUTF8Char *symbolName)
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

            if (!CLStringCompare8(symbolName, strings + symbol->nameOffset))
            {
                SLDebugPrint("Found symbol '%s' at %p\n", strings + symbol->nameOffset, symbol->value);

                return (OSAddress)(symbol->value + file->loadAddress);
            }
        }

        symbol++;
    }

    return kOSNullPointer;
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

            if (!CLStringCompare8(name, strings + symbol->nameOffset))
            {
                SLDebugPrint("Calling function '%s' at %p\n", strings + symbol->nameOffset, symbol->value);

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

    SLDebugPrint("Calling entry point at %p...\n", entryPoint);
    SLDebugPrint("Note: WILL switch stack to %p\n", stack);

    asm __volatile__ ("movq %0, %%rsp ;"
                      "movq %0, %%rbp ;"
                      "jmp *%1" : :
                      "r" (stack),
                      "r" (entryPoint),
                      "D" (file)
                      );

    SLPrintString("Error: ????\n");
    SLLeave(kSLStatusLoadError);
}
