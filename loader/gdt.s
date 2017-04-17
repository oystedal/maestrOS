.text

.align 8

.equ DESCRIPTOR_SIZE, 8
.equ DESCRIPTOR_COUNT, 3
.equ GDT_LIMIT,(DESCRIPTOR_COUNT * DESCRIPTOR_SIZE) - 1

.global gdtr
gdtr:
    .word  GDT_LIMIT # limit
    .long  gdt       # base address

.global gdt
gdt:
    # the first entry of the GDT is not used by the processor
    .long  0
    .long  0

    # code
    .word  0xffff # limit
    .word  0x0    # base 15:00
    .byte  0x0    # base 23:16
    .word  0xc09a # flags
    .byte  0x0    # base 31:24

    # data
    .word  0xffff # limit
    .word  0x0    # base 15:00
    .byte  0x0    # base 23:16
    .word  0xc092 # flags
    .byte  0x0    # base 31:24

.globl gdtr64
gdtr64:
    .word GDT_LIMIT
    .long gdt64

.globl gdt64
gdt64:
    .word 0
    .word 0
    .byte 0
    .byte 0
    .byte 0
    .byte 0

    # Code
    .word 0x0
    .word 0
    .byte 0
    .byte 0x9a
    .byte 0x20
    .byte 0x0

    # Data
    .word 0x0
    .word 0
    .byte 0
    .byte 0x92
    .byte 0x0
    .byte 0x0

