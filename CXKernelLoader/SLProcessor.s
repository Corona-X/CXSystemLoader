#include <Kernel/XKAssemblyCode.h>

#define kSLEarlyCodeSegment 0x08
#define kSLEarlyDataSegment 0x10

// Boot PML4 table declared in SLBootPageTables.c
.extern XKSymbol(_SLBootPML4)

// Boot PDPT table declared in SLBootPageTables.c
.extern XKSymbol(_SLBootPDPT)

.section kXKCodeSectionName

XKDeclareFunction(SLProcessorEarlyInit):
    pushq %rbp                                          // Setup stack frame
    movq %rsp, %rbp                                     // ^^

    // There's a loop here. No it doesn't have to be written in assembly.
    // Yes, the C compiler could probably do a better job writing this loop.
    // Yes, it could be more optimized, but we're literally copying like 100 values into memory, so it's not super critical...

    leaq XKSymbol(SLEarlyTrapHandler)(%rip), %rax       // Locate early trap handler
    movq %rax, %rcx                                     // Store handler pointer in rcx
    movq %rax, %rdx                                     // Store handler pointer in rdx

    shrq $32, %rax                                      // Shift %rax so it holds the high 32 bits of the handler pointer
    andq $0xFFFF, %rcx                                  // Mask off high 48 bits of rcx
    shrq $16, %rdx                                      // Shift off low 16 bits from rdx
    andq $0xFFFF, %rdx                                  // rdx now holds bits 16-31 of the pointer

    leaq XKSymbol(_SLProcessorBootIDT)(%rip), %rdi      // Get a pointer to our IDT
    movq %rdi, %rsi                                     // Copy pointer to our IDT
    addq $512, %rdi                                     // Start our loop counter

    // We don't know where this binary is in memory, so we have to manually fill the boot IDT
    //   with the proper values relative to %rip (in %rax from above)
    1:
        movq %rax, -0x08(%rdi)                          // Copy high 32 bits of handler pointer
        movw %dx, -0x0A(%rdi)                           // Copy next 16 bits
        subq $16, %rdi                                  // Move to next entry
        movw %cx, (%rdi)                                // Copy last 16 bits
        cmpq %rsi, %rdi                                 // Compare %rsi to %rdi
        jne 1b                                          // Continue if %rsi != %rdi

    // We setup the base address of our third and fourth level page tables here
    leaq XKSymbol(_SLBootPML4)(%rip), %rax              // Load the fourth level table pointer to rax
    addq %rax, 0xFF8(%rax)                              // Rebase the last entry in the table
    addq %rax, 0x000(%rax)                              // Rebase table to correct physical address

    leaq XKSymbol(_SLBootPDPT)(%rip), %rcx              // Load the third level page table pointer to rcx
    addq %rax, 0x00(%rcx)                               // Rebase physical address of PDE0
    addq %rax, 0x08(%rcx)                               // Rebase physical address of PDE1
    addq %rax, 0x10(%rcx)                               // Rebase physical address of PDE2
    addq %rax, 0x18(%rcx)                               // Rebase physical address of PDE3

    cli                                                 // Mask interrupts

    leaq XKSymbol(_SLProcessorBootGDTR)(%rip), %rax     // Load address of GDTR to %rax
    leaq XKSymbol(_SLProcessorBootGDT)(%rip), %rcx      // Load address of GDT table to %rcx

    movq %rcx, 0x02(%rax)                               // Ensure the GDTR holds the right value
    lgdt (%rax)                                         // Load new GDT into the CPU

    movw $kSLEarlyDataSegment, %ax                      // 64-Bit Data Segment Selector
    movw %ax, %ds                                       // Reset ds selector
    movw %ax, %es                                       // Reset es selector
    movw %ax, %fs                                       // Reset fs selector
    movw %ax, %gs                                       // Reset gs selector

    pushq $kSLEarlyDataSegment                          // Push new SS value for after far return
    pushq %rbp                                          // Push new stack pointer for after far return

    pushfq                                              // Push flags for after far return

    pushfq                                              // Push flags to modify
    popq %rax                                           // Grab flags value
    andq $0xFFFFFFFFFFFFBFFF, %rax                      // Ensure NT not set (iret causes a #GP otherwise)
    pushq %rax                                          // Get ready to set new flags value
    popfq                                               // Set new flags value

    pushq $kSLEarlyCodeSegment                          // 64-Bit Code Segment Selector
    leaq jump_target(%rip), %rax                        // Load rip-relative address of return value
    pushq %rax                                          // We need to perform a far return here
    iretq                                               //   to reload with the new GDT

    // This is the target for the far return we setup above.
    jump_target:
        leaq XKSymbol(_SLProcessorBootIDTR)(%rip), %rax // We changed the GDT, so we should load our new IDT ASAP.
        leaq XKSymbol(_SLProcessorBootIDT)(%rip), %rcx  // Load the start address of the new IDT table to rcx.

        movq %rcx, 0x02(%rax)                           // Ensure the new IDTR holds the right value
        lidt (%rax)                                     // Do that immidietely. Like right here.

        xorq %rax, %rax                                 // Clear rax for the next instruction
        lldt %ax                                        // We don't use an LDT. Set it to the null segment.

        leaq XKSymbol(_SLBootPML4)(%rip), %rax          // Get the physical address of our fourth level page table
        movq %rax, %cr3                                 // Load our boot paging structure

        popq %rbp                                       // Cleanup our stack frame
        retq                                            // That's it for now...

XKDeclareFunction(SLEarlyTrapHandler):
    hlt

.section __BOOT,__data
.align 8

XKDeclareGlobal(_SLProcessorBootGDT):
    // Null Segment Entry
    .quad 0x0000000000000000

    // First Segment Entry (64-Bit Code)
    .word 0xFFFF        // Limit 0...15
    .word 0x0000        // Base  0...15
    .byte 0x00          // Base  16..23
    .byte 0x9A          // Split:
                        // (4) Type : Seg. Type  [= 0xA]
                        // (1) S    : Sys/C/D    [= 1]
                        // (2) DPL  : Priviledge [= 0]
                        // (1) P    : Present    [= 1]
    .byte 0xAF          // Split:
                        // (4) Limit 16...19     [= 0xF]
                        // (1) AVL : Unused      [= 0]
                        // (1) L   : 64-Bit Code [= 1]
                        // (1) D/B : Oper. Size  [= 0]
                        // (1) G   : Granularity [= 1]
    .byte 0x00          // Base  24..31

    // Second Segment Entry (64-Bit Data)
    .word 0xFFFF        // Limit 0...15
    .word 0x0000        // Base  0...15
    .byte 0x00          // Base  16..23
    .byte 0x92          // Split:
                        // (4) Type : Seg. Type  [= 0x2]
                        // (1) S    : Sys/C/D    [= 1]
                        // (2) DPL  : Priviledge [= 0]
                        // (1) P    : Present    [= 1]
    .byte 0x8F          // Split:
                        // (4) Limit 16...19     [= 0xF]
                        // (1) AVL : Unused      [= 0]
                        // (1) L   : 64-Bit Code [= 0]
                        // (1) D/B : Oper. Size  [= 0]
                        // (1) G   : Granularity [= 1]
    .byte 0x00          // Base  24..31

    // That's all we need since we exclusively use 64-Bit mode

.align 12

// Some notes on these values:
// -- The first and second words, as well as the first quad word, are filled at runtime with a pointer to the trap handlers
// -- The second word is the code segment of the handler. We only have a single code segment in our GDT as per above.
// -- The first byte only represents the IST offset in 64-Bit mode. We don't use an IST here, so we just use 0.
// -- The second byte has a few flags. The low 4 bytes are the type (0xE here) and the high bit indicates the segment is present.
// -- The last quadword is reserved; we set it to 0.
#define SLDeclareIDTEntry()         \
    .word 0x0000                  ; \
    .word kSLEarlyCodeSegment     ; \
    .byte 0x00                    ; \
    .byte 0x8E                    ; \
    .word 0x0000                  ; \
    .long 0x00000000              ; \
    .long 0x00000000

// Interrupts are disabled until we have a real IDT.
// Catch all traps that occur above to avoid triple faulting
XKDeclareGlobal(_SLProcessorBootIDT):
    SLDeclareIDTEntry(); SLDeclareIDTEntry(); SLDeclareIDTEntry(); SLDeclareIDTEntry();
    SLDeclareIDTEntry(); SLDeclareIDTEntry(); SLDeclareIDTEntry(); SLDeclareIDTEntry();
    SLDeclareIDTEntry(); SLDeclareIDTEntry(); SLDeclareIDTEntry(); SLDeclareIDTEntry();
    SLDeclareIDTEntry(); SLDeclareIDTEntry(); SLDeclareIDTEntry(); SLDeclareIDTEntry();
    SLDeclareIDTEntry(); SLDeclareIDTEntry(); SLDeclareIDTEntry(); SLDeclareIDTEntry();
    SLDeclareIDTEntry(); SLDeclareIDTEntry(); SLDeclareIDTEntry(); SLDeclareIDTEntry();
    SLDeclareIDTEntry(); SLDeclareIDTEntry(); SLDeclareIDTEntry(); SLDeclareIDTEntry();
    SLDeclareIDTEntry(); SLDeclareIDTEntry(); SLDeclareIDTEntry(); SLDeclareIDTEntry();

// Align the pointer below
.balign 8
.skip 6

XKDeclareGlobal(_SLProcessorBootIDTR):
    .word 512 // 16 * 32 Trap Gates
    .quad XKSymbol(_SLProcessorBootIDT)

// Align the pointer below
.balign 8
.skip 6

// A pointer to the above GDT
XKDeclareGlobal(_SLProcessorBootGDTR):
    .word 24 // 8 * 3 Segment Entries
    .quad XKSymbol(_SLProcessorBootGDT)
