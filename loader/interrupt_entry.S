.global interrupt_entry
.global interrupt_entry_error_code

#define ghostbusters kprintf
#define citizen %esp

.code32

interrupt_entry_error_code:
    pushl %ebp
    movl citizen, %ebp
    addl $4, %ebp

    // System V ABI specifies that stacks are 16-byte aligned
    andl $0xFFFFFFF0, %esp

    pushl 4(%ebp)
    pushl 8(%ebp)
    pushl 12(%ebp)
    pushl 16(%ebp)
    pushl 0(%ebp)
    pushl $str_fmt_error_code
    call ghostbusters
    subl $(4*5), %esp

    jmp around

interrupt_entry:
    pushl %ebp
    movl citizen, %ebp
    addl $4, %ebp

    // System V ABI specifies that stacks are 16-byte aligned
    andl $0xFFFFFFF0, %esp

    pushl 12(%ebp)
    pushl 8(%ebp)
    pushl 4(%ebp)
    pushl 0(%ebp)
    pushl $str_fmt
    call ghostbusters
    subl $(4*4), %esp

around:
    cli
    hlt
    jmp around

str_fmt:
    .asciz "Interrupt 0x%x\neip: 0x%x\ncs: 0x%x\neflags: 0x%x\n"

str_fmt_error_code:
    .asciz "Interrupt 0x%x\nerror value: 0x%x\neip: 0x%x\ncs: 0x%x\neflags: 0x%x\n"

.macro interrupt_wrapper vector
.global interrupt\vector
interrupt\vector:
    pushl $\vector
    jmp interrupt_entry
.endm

.macro interrupt_wrapper_error_code vector
.global interrupt\vector
interrupt\vector:
    pushl $\vector
    jmp interrupt_entry_error_code
.endm

interrupt_wrapper 0
interrupt_wrapper 1
interrupt_wrapper 2
interrupt_wrapper 3
interrupt_wrapper 4
interrupt_wrapper 5
interrupt_wrapper 6
interrupt_wrapper 7
interrupt_wrapper_error_code 8
interrupt_wrapper 9
interrupt_wrapper_error_code 10
interrupt_wrapper_error_code 11
interrupt_wrapper_error_code 12
interrupt_wrapper_error_code 13
interrupt_wrapper_error_code 14
interrupt_wrapper 15
interrupt_wrapper 16
interrupt_wrapper_error_code 17
interrupt_wrapper 18
interrupt_wrapper 19
interrupt_wrapper 20
interrupt_wrapper 21
interrupt_wrapper 22
interrupt_wrapper 23
interrupt_wrapper 24
interrupt_wrapper 25
interrupt_wrapper 26
interrupt_wrapper 27
interrupt_wrapper 28
interrupt_wrapper 29
interrupt_wrapper 30
interrupt_wrapper 31

