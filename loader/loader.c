#include "stdint-gcc.h"
#include "multiboot.h"
#include "../util.h"
#include "interrupts.h"
#include "memory.h"

};

void halt(void);

void loader_main(uint32_t magic, uint32_t addr)
{
    (void)addr;

    __asm__ volatile ("lgdt gdtr");
    __asm__ volatile (
            "ljmp $0x8, $reload_cs;"
            "reload_cs:"
            "mov $0x10, %eax;"
            "mov %eax, %ds;"
            "mov %eax, %es;"
            "mov %eax, %fs;"
            "mov %eax, %gs;"
            "mov %eax, %ss;"
    );

    init_idt();

    // clear screen
    cls();

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        kprintf("Incorrect multiboot magic.\n");
        kprintf("Expected 0x%x - Actual: 0x%x\n",
                MULTIBOOT_BOOTLOADER_MAGIC,
                magic);
        goto end;
    }

    kprintf("Multiboot magic OK\n");

    early_init_memory();
    enable_paging();

end:
    kprintf("End of loader_main.\n");
    halt();
}

void halt(void)
{
    kprintf("Halting.\n");
    BXBREAK;
    for (;;) {
        __asm__ volatile ("cli");
        __asm__ volatile ("hlt");
    }
}
