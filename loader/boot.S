#define ASM_FILE
#include "multiboot.h"

#ifndef __ELF__
#error "Loader must be compiled as an elf binary"
#endif

// #define ENABLE_ALIGN_4K (1 << 0)
// #define INCLUDE_MEMORY_INFO (1 << 1)
// #define INCLUDE_VIDEO_INFO (0 << 2)
// 
// #define MULTIBOOT_HEADER_FLAGS (ENABLE_ALIGN_4K + INCLUDE_MEMORY_INFO + INCLUDE_VIDEO_INFO)

#define MULTIBOOT_HEADER_FLAGS         0x00000003

.text

.code32

.global _start

_start:
    jmp entry

// Align 32 bits boundary.
.align 4

multiboot_header:
// Multiboot magic
.long MULTIBOOT_HEADER_MAGIC
// Flags
.long MULTIBOOT_HEADER_FLAGS
// Checksum
.long -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

entry:
    // Stack
    movl $(stack + 0x0ff0), %esp

    // EFLAGS reset
    pushl $0
    popf

    // Push Multiboot header pointer 
    pushl %ebx

    // Push multiboot magic value
    pushl %eax

    // Come on, let's do this!
    call loader_main

    // TODO: Print some message

up_and_down:
    cli
    hlt
    jmp up_and_down

.section .bss, "aw"

.align 0x1000

stack:
    .skip 0x1000

// Directory/table for lower 1GB
.global page_directory
page_directory:
    .skip 0x1000
.global page_table
page_table:
    .skip 0x1000

// Directory/table for loader
.global pdt1
pdt1:
    .skip 0x1000

.global pt1
pt1:
    .skip 0x1000
