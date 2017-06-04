
.globl yield
.globl dispatch

.macro save_gp_regs
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
.endm

.macro restore_gp_regs
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
.endm

.macro save_fp_regs
    subq $112, %rsp
    fnsave 0(%rsp)
.endm

.macro restore_fp_regs
    frstor 0(%rsp)
    addq $112, %rsp
.endm

yield:
    pushfq
    save_gp_regs
    save_fp_regs

    mov current_task, %rax

    subq $8, %rsp
    mov %rsp, 0(%rax)
    addq $8, %rsp

    call schedule

    restore_fp_regs
    restore_gp_regs
    popfq

    ret

dispatch:
    mov current_task, %rax

    cmpq $0, 8(%rax)
    jz set_initialized

    ret
set_initialized:
    movq $1, 8(%rax)
    xorq %rax, %rax
    ret
