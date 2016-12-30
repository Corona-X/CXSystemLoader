#include <Kernel/XKAssembly.h>
#include <SystemLoader/SLSerial.h>

#if kCXArchIA && kCXBuildDev

// About the stack setup for these functions...
// They don't have any!! (Except 2 of them)
// That means that if there is an error in any of these routines,
// it will not be visible in the stack trace, but there routines
// don't call out anywhere external and they're generally too simple
// to really cause many errors so it should be alright...

.section kXKCodeSectionName
.align kXKNaturalAlignment

.data

test_string_1:
    .asciz "Read 0x%02X\n"

test_string_2:
    .asciz "Expected 0x%02X\n"

.text

// Arguments:
//   %dx: scratch register
//
// Return Value:
//   failure: Jump to \fail
//   success: Continue execution
//
// Destroyed Registers:
//   %al
.macro SLSerialPortTestValue value, fail
    movb \value, %al                    // Load test value into accumulator
    outb %al, %dx                       // Send test value to scratch space
    inb %dx, %al                        // Read value in scratch space
    cmpb \value, %al                    // Check if value was saved correctly
    jne \fail                           // Jump to failure handler if it wasn't
.endm

// Arguments:
//   %esi: port base
//
// Return Value:
//   failure: Jump to \fail
//   success: Continue execution
//
// Destroyed Registers:
//   %al
//   %dx
.macro SLSerialPortTest fail
    leaw 7(%esi), %dx                   // Load address of scratch register (base + 7) to I/O register (%dx)
    SLSerialPortTestValue $0xC7, \fail  // Test with first value
    SLSerialPortTestValue $0x83, \fail  // test with second value
.endm

// Arguments:
//   \wr: temporary register
//   \times: number of times to loop
//
// Return Value:
//   ---
//
// Destroyed Registers:
//   \wr
.macro SLSerialDelay wr, times
    movw \times, \wr                    // Move counter to loop counter

    99:
        decw \wr                        // Decrement loop counter
        jnz 99b                         // Again if we're not done

    // The reason why this loop should delay enough is that the CPU
    // *should* be tricked into throwing out its pipeline each time
    // the jump instruction is executed. I can't say for sure weather
    // or not this actually occurs as I haven't tested it, but I don't
    // think the branch prediction works well enough to see through my
    // tricks. (hopefully?)
.endm

// Arguments:
//   %di: port base
//
// Return Value (%ax):
//   portBase: success
//   kSLSerialPortError: failure
//
// Destroyed Registers:
//   %al
//   %dx
//   %esi
XKDeclareFunction(SLSerialPortInit):
    movzwl %di, %esi                    // Save port address to %esi
    SLSerialPortTest 1f                 // Check if serial port exists; return otherwise
    callq XKSymbol(SLSerialPortReset)   // Reset Port (%di destroyed)
    leaw 4(%esi), %dx                   // Load modem control register (base + 4) to I/O port register (%dx)
    movb $0x02, %al                     // Load initial modem control register value
    outb %al, %dx                       // Send initial modem control register value to modem control port
    subw $4, %dx                        // Move %dx back to port base
    inb %dx, %al                        // Clear character buffer
    movw %dx, %ax                       // Load port intro return value
    ret                                 // Return port address (success)

    1:
        movw $kSLSerialPortError, %ax   // Load failure return value
        ret                             // Return error address

// Arguments:
//   %di: port base
//
// Return Value:
//   ---
//
// Destroyed Registers:
//   %al
//   %dx
//   %edi
XKDeclareFunction(SLSerialPortReset):
    movzwl %di, %edi                    // Save port address to %edi
    leaw 3(%edi), %dx                   // Load line control register (base + 3) to I/O port register (%dx)
    movb $0x80, %al                     // Load value to expose divisor registers into accumulator
    outb %al, %dx                       // Expose divisor registers
    leaw (%edi), %dx                    // Load port base into I/O register
    movb $1, %al                        // Load LSB of divisor into accumulator
    outb %al, %dx                       // Reset LSB of divisor to 1
    xorb %al, %al                       // Clear accumulator
    incw %dx                            // Increment I/O register to point to MSB of divisor
    outb %al, %dx                       // Reset MSB of divisor to 0
    leaw 3(%edi), %dx                   // Load line control register to I/O port register
    outb %al, %dx                       // Reset line control to 0
    incw %dx                            // Increment I/O register to point to modem control register
    outb %al, %dx                       // Reset modem control register to 0
    incw %dx                            // Increment I/O register to point to line status register
    movb $0x60, %al                     // Load default line status into the accumulator
    outb %al, %dx                       // Reset line status to 0x60
    incw %dx                            // Increment I/O register to point to modem status register
    inb %dx, %al                        // Load current value of modem status register
    andb $0xF0, %al                     // Unset low 4 bits of modem status register (in accumulator)
    outb %al, %dx                       // Reset modem status register
    xorb %al, %al                       // Clear accumulator
    leaw 1(%edi), %dx                   // Load interrupt enable register (base + 1) into I/O port register
    outb %al, %dx                       // Reset interrupt enable register to 0 (All interrupts disabled)
    incw %dx                            // Increment I/O register to point to FIFO control register
    outb %al, %dx                       // Reset FIFO control register to 0 (FIFO disabled)
    ret                                 // All done; return to parent

// Arguments:
//   %di: port
//   %sil: word length
//   %cl: parity
//   %dl: stop bits
//
// Return Value:
//   ---
//
// Destroyed Registers:
//   %al
//   %dx
XKDeclareFunction(SLSerialPortSetupLineControl):
    movb %cl, %al                       // Store parity bits in accumulator
    shlb $3, %al                        // Move parity bits to correct position in accumulator
    shlb $1, %dl                        // Move stop bit to correct position
    orb %dl, %al                        // Or stop bit into accumulator
    orb %sil, %al                       // Or word length bits into accumulator
    leaw 3(%edi), %dx                   // Load line control register (base + 3) into I/O register (%dx)
    outb %al, %dx                       // Send new line control value to line control register
    ret                                 // All done; return to parent

// Arguments:
//   %di: port
//   %si: divisor
//
// Return Value:
//   ---
//
// Destroyed Registers:
//   %ax
//   %cl
//   %dx
XKDeclareFunction(SLSerialPortSetBaudDivisor):
    leaw 3(%edi), %dx                   // Load line control register (base + 3) into I/O register (%dx)
    inb %dx, %al                        // Load current line control register into accumulator
    movb %al, %cl                       // Store origin line control value
    orb $0x80, %al                      // Enable bit which exposes divisor in accumulator
    outb %al, %dx                       // Output divisor access bit into line control register
    leaw (%edi), %dx                    // Load divisor LSB (base + 0) into I/O register
    movb %sil, %al                      // Load divisor LSB into accumulator
    outb %al, %dx                       // Output divisor LSB
    incw %dx                            // Increment I/O register to point to divisor MSB
    movw %si, %ax                       // Move full divisor to accumulator
    movb %ah, %al                       // Move MSB of divisor to the low 8 bits of the accumulator
    outb %al, %dx                       // Output divisor MSB
    leaw 3(%edi), %dx                   // Load line control register into I/O register
    movb %cl, %al                       // Load saved line control value back into accumulator
    outb %al, %dx                       // Output saved line control value
    ret                                 // All done; return to parent

// Arguments:
//   %di: port
//   %si: rate
//
// Return Value:
//   ---
//
// Destroyed Registers:
//   %ax
//   %cl
//   %dx
//   %si
XKDeclareFunction(SLSerialPortSetBaudRate):
    testw %si, %si                      // Check to see if the rate is 0
    jz 1f                               // If it is, get out and avoid a division error

    movw $0xC200, %ax                   // Load clock rate LSB into the accumulator
    movw $0x1, %dx                      // Load clock rate MSB into extended accumulator
    divw %si                            // Divide by provided rate
    test %dx, %dx                       // Test for a remainder
    jnz 1f                              // Return if there was a remainder
    movw %ax, %si                       // Load divisor into second argument register
    callq XKSymbol(SLSerialPortSetBaudDivisor)  // Set the divisor

    1:
        ret                             // We either did it or an error occurred; return in either case

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
//   0x00: failure
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
        movb $0x0, %al
        ret

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
