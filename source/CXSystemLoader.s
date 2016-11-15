#include <Kernel/CXKAssembly.h>

#if kCXArchIA

.section kCXKCodeSectionName
.align kCXKNaturalAlignment

// Arguments:
//   %rcx: Image Handle
//   %rdx: System Table
//
// Return Value (%rax):
//   0: success
//   other: failure
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
//
CXKDeclareFunction(_SLEntry):
    movq (%rsp), %rax

    CXKLoadSymbol(gSLFirmwareReturnAddress, %rbx)
    movq %rax, (%rbx)

    CXKLoadSymbol(gSLLoaderImageHandle, %rbx)
    movq %rcx, (%rbx)

    CXKLoadSymbol(gSLLoaderSystemTable, %rbx)
    movq %rdx, (%rbx)

    #if kCXBuildDev && !kCXTargetOSApple
        leaq gSLLoaderBase(%rip), %rbx
        leaq gSLLoaderEnd(%rip), %rax
        subq %rbx, %rax
        CXKLoadSymbol(gSLLoaderImageSize, %rbx)
        movq %rax, (%rbx)

        CXKLoadSymbol(gSLEnableScreenPrint, %rbx)
        movb $1, (%rbx)
    #endif /* kCXBuildDev */

    CXKLoadSymbol(gSLBootServicesEnabled, %rbx)
    movb $1, (%rbx)

    leaq CXKFunction(SLMemoryAllocatorInit)(%rip), %rax
    callq *%rax

    #if kCXBuildDev
        leaq CXKFunction(__SLLibraryInitialize)(%rip), %rax
        callq *%rax
    #endif /* kCXBuildDev */

    CXKLoadSymbol(gSLLoaderImageHandle, %rcx)
    CXKLoadSymbol(gSLLoaderSystemTable, %rdx)

    leaq CXKFunction(CXSystemLoaderMain)(%rip), %rax
    callq *%rax

    movq %rax, %rcx
    leaq CXKFunction(SLLeave)(%rip), %rax
    callq *%rax

.align kCXKNaturalAlignment

// Arguments:
//   %rcx: Exit Code
//
// This function will exit the loader
// without returning. It can do pretty
// much whatever it wants...
CXKDeclareFunction(SLLeave):
    movq %rcx, %rdx
    CXKLoadSymbol(gSLLoaderImageHandle, %rbx)
    movq (%rbx), %rcx
    xorq %r8, %r8
    xorq %r9, %r9
    CXKLoadSymbol(gSLLoaderSystemTable, %rax)
    movq (%rax), %rbx
    movq 0x60(%rbx), %rax
    callq *0xD8(%rax)

    // If we're here, there was an error exiting.
    // Return to the address given by the firmware
    // on the initial program stack (saved in entry)
    CXKLoadSymbol(gSLFirmwareReturnAddress, %rbx)
    movq (%rbx), %rax // Load return address into accumulator
    popq %rbx         // Kill real return address on the stack
    pushq (%rax)      // Inject firmware return address
    ret               // Return to given address

.comm CXKSymbol(gSLFirmwareReturnAddress), 8, 8
.comm CXKSymbol(gSLLoaderSystemTable),     8, 8
.comm CXKSymbol(gSLLoaderImageHandle),     8, 8
.comm CXKSymbol(gSLBootServicesEnabled),   1, 1

#if kCXBuildDev
    .comm CXKSymbol(gSLLoaderImageSize),   8, 8
    .comm CXKSymbol(gSLEnableScreenPrint), 1, 1
#endif /* kCXBuildDev */

#if kCXTargetOSLinux
.section .reloc, "a", @progbits

// Pretend like we're a relocatable executable...
.int 0
.int 10
.word 0
#endif /* Target OS */

#endif /* Arch */
