#include <Kernel/XKAssembly.h>

#if kCXArchIA

.section kXKCodeSectionName
.align kXKNaturalAlignment

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

    #if kCXBuildDev && !kCXTargetOSApple
        leaq gSLLoaderBase(%rip), %rbx                      // If custom link, load the base address of the loader into the b register
        leaq gSLLoaderEnd(%rip), %rax                       // If custom link, load the end address of the loader into the accumulator
        subq %rbx, %rax                                     // Calculate the size of the loader in memory
        XKLoadSymbol(gSLLoaderImageSize, %rbx)              // Load the pointer to the size of the loader into the b register
        movq %rax, (%rbx)                                   // Store the size of the loader into memory
    #endif /* kCXBuildDev */

    XKLoadSymbol(gSLBootServicesEnabled, %rbx)              // Load the BootServicesEnabled variable pointer into the b register
    movb $1, (%rbx)                                         // Mark boot services as being enabled

    #if kCXBuildDev
        leaq XKSymbol(__SLLibraryInitialize)(%rip), %rax    // Load the addres of the development library initialze routine into the accumulator
    #else /* !kCXBuildDev */
        leaq XKSymbol(SLMemoryAllocatorInit)(%rip), %rax    // Load the address of the memory allocator initialze function into the accumulator
    #endif /* kCXBuildDev */

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
    movq %rcx, %rdx
    XKLoadSymbol(gSLLoaderImageHandle, %rbx)
    movq (%rbx), %rcx
    xorq %r8, %r8
    xorq %r9, %r9
    XKLoadSymbol(gSLLoaderSystemTable, %rax)
    movq (%rax), %rbx
    movq 0x60(%rbx), %rax
    callq *0xD8(%rax)

    // If we're here, there was an error exiting.
    // Return to the address given by the firmware
    // on the initial program stack (saved in entry)
    XKLoadSymbol(gSLFirmwareReturnAddress, %rbx)
    movq (%rbx), %rax // Load return address into accumulator
    popq %rbx         // Kill real return address on the stack
    pushq (%rax)      // Inject firmware return address
    ret               // Return to given address

.comm XKSymbol(gSLFirmwareReturnAddress), 8, 8
.comm XKSymbol(gSLLoaderSystemTable),     8, 8
.comm XKSymbol(gSLLoaderImageHandle),     8, 8
.comm XKSymbol(gSLBootServicesEnabled),   1, 1

#if kCXBuildDev
    .comm XKSymbol(gSLLoaderImageSize),   8, 8
    .comm XKSymbol(gSLEnableScreenPrint), 1, 1
#endif /* kCXBuildDev */

#if kCXTargetOSLinux
.section .reloc, "a", @progbits

// Pretend like we're a relocatable executable...
.int 0
.int 10
.word 0
#endif /* Target OS */

#endif /* Arch */
