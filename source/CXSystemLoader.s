#include <Kernel/XKAssemblyCode.h>

#if kCXArchIA

.section kXKCodeSectionName

.extern SKSymbol(gSLConsoleIsInitialized)
.extern SKSymbol(SLPrintBufferFlush)

// Arguments:
//   %rcx: Image Handle
//   %rdx: System Table
//
// Return Value:
//   This function does not return.
//
// Destroyed Registers:
//   %rax
//   %rbx
//   %rcx
//   %rdx
//   %rdi
//   %rsi
//
// If Debug:
//   ---
XKDeclareFunction(_SLEntry):
    movq (%rsp), %rax                                       // Copy the return address passed on the stack to the accumulator

    XKLoadSymbol(gSLFirmwareReturnAddress, %rbx)            // Load the pointer to the firmware return address symbol into the b register
    movq %rax, (%rbx)                                       // Store the return address for the firmware into memory

    XKLoadSymbol(gSLLoaderImageHandle, %rbx)                // Load the pointer to the loader image handle into the b register
    movq %rcx, (%rbx)                                       // Store the image handle passed as first argument (fastcall ABI, %rcx) into memory

    XKLoadSymbol(gSLLoaderSystemTable, %rbx)                // Load the pointer to the system table into the b register
    movq %rdx, (%rbx)                                       // Store the system table pointer passed as second argument (fastcall ABI, %rdx) into memory

    XKLoadSymbol(gSLBootServicesEnabled, %rbx)              // Load the BootServicesEnabled variable pointer into the b register
    movb $1, (%rbx)                                         // Mark boot services as being enabled

    leaq XKSymbol(SLConsoleInitialize)(%rip), %rax          // Load the address of the BootConsole initialize function into the accumulator
    callq *%rax                                             // Initialize the BootConsole here

    leaq XKSymbol(SLMemoryAllocatorInit)(%rip), %rax        // Load the address of the memory allocator initialze function into the accumulator
    callq *%rax                                             // Call the address stored in the accumulator (depends on if a development build)

    XKLoadSymbol(gSLLoaderImageHandle, %rbx)                // Load the pointer to the loader's image handle into the b register
    movq (%rbx), %rdi                                       // Load the loader's image handle into the first argument register (SysV ABI, %rdi)

    XKLoadSymbol(gSLLoaderSystemTable, %rbx)                // Load the pointer to the system table pointer into the b register
    movq (%rbx), %rsi                                       // Load the system table pointer into the second argument register (SysV ABI, %rsi)

    leaq XKSymbol(CXSystemLoaderMain)(%rip), %rax           // Load the address of the C entry point into the accumulator
    callq *%rax                                             // Call into the C entry point of the loader

    movq %rax, %rcx                                         // Use the return address from the main function as the first argument to SLLeave
    pushq %rax                                              // Push a fake return address. SLLeave will inject firmware return address here on the stack

// Arguments:
//   %rcx: Exit Code
//
// This function will exit the loader
// without returning. It can do pretty
// much whatever it wants...
XKDeclareFunction(SLLeave):
    XKLoadSymbol(gSLConsoleIsInitialized, %rbx)             // Load Address of symbol which indiccates if BootConsole has been initialized
    movq (%rbx), %rax                                       // Load value into accumulator
    test %rax, %rax                                         // Test if boot console has been initialized
    jz 1f                                                   // If boot console wasn't initialized, just exit

    leaq XKSymbol(SLPrintBufferFlush)(%rip), %rax           // Prepare to flush the print buffer
    callq *%rax                                             // Flush the print buffer

    1:
        movq %rcx, %rdx                                     // Save exit code in the d register
        XKLoadSymbol(gSLLoaderImageHandle, %rbx)            // Load the EFI image handle pointer
        movq (%rbx), %rcx                                   // Store the image handle in the c register
        xorq %r8, %r8                                       // Clear 3rd argument
        xorq %r9, %r9                                       // Clear 4th argument
        XKLoadSymbol(gSLLoaderSystemTable, %rax)            // Load the EFI system table pointer
        movq (%rax), %rbx                                   // Store the system table into the accumulator
        movq 0x60(%rbx), %rax                               // Locate the address of the boot services table
        callq *0xD8(%rax)                                   // Call the boot services exit function

        // If we're here, there was an error exiting.
        // Return to the address given by the firmware
        // on the initial program stack (saved in entry)
        XKLoadSymbol(gSLFirmwareReturnAddress, %rbx)
        movq (%rbx), %rax                                   // Load return address into accumulator
        popq %rcx                                           // Kill real return address on the stack
        pushq (%rax)                                        // Inject firmware return address
        ret                                                 // Return to given address

// Arguments:
//   %rcx: The number of pages to allocate
//
// Return Value:
//   The value returned from SLBootServicesAllocateAnyPages
//
// Destroyed Registers:
//
// This function simply calls through to SLBootServicesAllocateAnyPages.
// When kernel loader maps the kernel into memory, boot services are not guarenteed to be active.
// As such, we need to give SLMach-O an implementation-agnostic function to allocate pages.
XKDeclareFunction(SLAllocatePages):
    leaq XKSymbol(SLBootServicesAllocateAnyPages)(%rip), %rax   // Load the address of the boot services' alloc function
    callq *%rax                                                 // Call through
    ret                                                         // Immidietely return. Pass return value through.

// Arguments:
//   %rcx: The start of the pages in memory
//   %rdx: The number of pages to free
//
// Return Value:
//   None
//
// Destroyed Registers:
//
// This function simply calls through to SLBootServicesFreePages.
// When kernel loader maps the kernel into memory, boot services are not guarenteed to be active.
// As such, we need to give SLMach-O an implementation-agnostic function to allocate pages.
XKDeclareFunction(SLFreePages):
    leaq XKSymbol(SLBootServicesFreePages)(%rip), %rax   // Load the address of the boot services' free function
    jmpq *%rax                                           // Boing boing!

.comm XKSymbol(gSLFirmwareReturnAddress), 8, 8
.comm XKSymbol(gSLLoaderSystemTable),     8, 8
.comm XKSymbol(gSLLoaderImageHandle),     8, 8
.comm XKSymbol(gSLBootServicesEnabled),   1, 1

#endif /* Arch */
