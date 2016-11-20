#include <Kernel/XKAssembly.h>

#define testValue1 0xC7
#define testValue2 0x83

#if kCXArchIA && kCXBuildDev

.section kXKCodeSectionName
.align kXKNaturalAlignment

// Arguments:
//   %cx: port
//
// Return Value (ZF):
//   0: failure
//   1: success
//
// Destroyed Registers:
//   %al
//   %dx
.macro SLSerialPortTest fail
    movb $testValue1, %al
    movw %cx, %dx
    addw $7, %dx
    outb %al, %dx
    inb %dx, %al
    xorb $testValue1, %al
    jnz \fail
    movb $testValue2, %al
    outb %al, %dx
    inb %dx, %al
    xorb $testValue2, %al
    jnz 1f
    xorb %al, %al
.endm

// Arguments:
//   %cl: time
//
// Return Value:
//   ---
//
// Destroyed Registers:
//   \wr
.macro SLSerialDelay wr, times
    //callq SLBootServicesHaveTerminated
    //movb %al, %dl
    //callq SLDelayProcessor

    movw \times, \wr

    99:
        nop; nop; nop; nop
        nop; nop; nop; nop
        nop; nop; nop; nop
        nop; nop; nop; nop
        decw \wr
        jnz 99b
.endm

// Arguments:
//   %cx: portBase
//
// Return Value (%ax):
//   portBase: success
//   0xFFFF: failure
//
// Destroyed Registers:
//   %dx
//   %al
XKDeclareFunction(SLSerialPortInit):
    SLSerialPortTest 1f
    pushw %cx
    callq XKSymbol(SLSerialPortReset)
    popw %cx
    movw %cx, %dx
    addw $4, %dx
    movb $0x02, %al
    outb %al, %dx
    movw %cx, %dx
    inb %dx, %al
    movw %cx, %ax
    ret

    1:
        movw $0xFFFF, %ax
        ret

// Arguments:
//   %cx: port
//
// Return Value:
//   ---
//
// Destroyed Registers:
//   %al
//   %cx
//   %dx
//   %r8w
XKDeclareFunction(SLSerialPortReset):
    movb $0x80, %al
    movw %cx, %dx
    addw $3, %dx
    movw %dx, %r8w
    outb %al, %dx
    movw %cx, %dx
    xorb %al, %al
    outb %al, %dx
    incw %dx
    movw %dx, %cx
    outb %al, %dx
    movw %r8w, %dx
    outb %al, %dx
    movw %cx, %dx
    outb %al, %dx
    incw %dx
    outb %al, %dx
    addw $2, %dx
    outb %al, %dx
    outb %al, %dx
    addw $2, %dx
    outb %al, %dx
    decw %dx
    inb %dx, %al
    andb $0xF0, %al
    outb %al, %dx
    ret

// Arguments:
//   %cx: port
//   %dl: word size
//   %r8b: parity
//   %r9b: stop bits
//
// Return Value:
//   ---
//
// Destroyed Registers:
//   %al
//   %dx
//   %r8b
//   %r9b
XKDeclareFunction(SLSerialPortSetupLineControl):
    movb %dl, %al
    shlb $3, %r8b
    orb %r8b, %al
    shlb $1, %r9b
    orb %r9b, %al
    movw %cx, %dx
    addw $3, %dx
    outb %al, %dx
    ret

// Arguments:
//   %cx: port
//   %dx: divisor
//
// Return Value:
//   ---
//
// Destroyed Registers:
//   %al
//   %dx
//   %r8w
//   %r9b
XKDeclareFunction(SLSerialPortSetBaudDivisor):
    movw %dx, %r8w
    movw %cx, %dx
    addw $3, %dx
    inb %dx, %al
    movb %al, %r9b
    orb $0x80, %al
    outb %al, %dx
    movw %cx, %dx
    movw %r8w, %ax
    outb %al, %dx
    shrw $8, %ax
    incw %dx
    outb %al, %dx
    addw $2, %dx
    movb %r9b, %al
    outb %al, %dx
    ret

// Arguments:
//   %cx: port
//   %edx: rate
//
// Return Value:
//   ---
//
// Destroyed Registers:
//   %eax
//   %edx
//   %r8d
XKDeclareFunction(SLSerialPortSetBaudRate):
    testl %edx, %edx
    jz 1f

    movl $0x1C200, %eax
    movl %edx, %r8d
    xorl %edx, %edx
    divl %r8d
    test %edx, %edx
    jnz 1f
    movl %eax, %edx
    callq XKSymbol(SLSerialPortSetBaudDivisor)

    1:
        ret

// Arguments:
//   %cx: port
//   %dl: character
//   %r8b: block
//
// Return Value:
//   ---
//
// Destroyed Registers:
//   %ax
//   %dx
//   %r9b
XKDeclareFunction(SLSerialWriteCharacter):
    movb %dl, %r9b

    cmpb $0x0A, %r9b
    jnz 1f
    movb $0x0D, %dl
    shlw $8, %r8w
    orb $1, %r8b
    callq XKSymbol(SLSerialWriteCharacter)
    movb $0x0A, %r9b
    shrw $8, %r8w

    1:
        movw %cx, %dx
        addw $5, %dx

    2:
        inb %dx, %al
        shrb $5, %al
        andb $1, %al
        jnz 3f
        testb %r8b, %r8b
        jz 4f
        SLSerialDelay %ax, $100
        jmp 2b

    3:
        movb %r9b, %al
        subw $5, %dx
        outb %al, %dx
        ret

    4:
        ret

// Arguments:
//   %cx: port
//   %dl: block
//
// Return Value (%al):
//   0xFF: failure
//   other: success
//
// Destroyed Registers:
//   %ax
//   %dx
//   %r8b
XKDeclareFunction(SLSerialReadCharacter):
    movb %dl, %r8b
    movw %cx, %dx
    addw $5, %dx

    1:
        inb %dx, %al
        shrb $1, %al
        andb $7, %al
        jnz 2f
        inb %dx, %al
        andb $1, %al
        jnz 2f
        testb %r8b, %r8b
        jz 3f
        SLSerialDelay %ax, $5
        jmp 1b

    2:
        subw $5, %dx
        inb %dx, %al
        ret

    3:
        movb $0xFF, %al

// Arguments:
//   %cx: port
//   %rdx: string pointer
//
// Return Value:
//   ---
//
// Destroyed Registers:
//   %r8b
//   %r10
//   %dl
XKDeclareFunction(SLSerialWriteString):
    movq %rdx, %r10
    movb $1, %r8b

    1:
        movb (%r10), %dl
        testb %dl, %dl
        jz 2f
        incq %r10
        callq XKSymbol(SLSerialWriteCharacter)
        jmp 1b

    2:
        ret

// Arguments:
//   %cx: port
//   %dl: terminator
//   %r8: buffer {address, size}
//   %r9: print
//
// Return Value:
//   ---
//
// Destroyed Registers:
//   %r10b
//   %r11
//   %r9
//   %dx
XKDeclareFunction(SLSerialReadString):
    cmpb $0x0A, %dl
    movw $0x000D, %r11w
    cmovzw %r11w, %dx

    pushq %rbx
    pushq %rdi

    movb %dl, %r10b
    movq 0x00(%r8), %rbx
    movq 0x08(%r8), %r11
    decq %r11

    1:
        movb $1, %dl
        callq XKSymbol(SLSerialReadCharacter)
        movb %al, %dil
        movb %al, %dl
        movb $1, %r8b
        callq XKSymbol(SLSerialWriteCharacter)
        cmpb %r10b, %dil
        je 2f
        movb %dil, (%rbx)
        incq %rbx
        decq %r11
        jnz 1b

    2:
        movb $0x00, (%rbx)
        popq %rdi
        popq %rbx
        ret

#elif kCXArchARM && kCXBuildDev

#endif /* Arch & Dev */
