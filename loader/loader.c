#include "stdint-gcc.h"
#include "multiboot.h"
#include "../util.h"
#include "interrupts.h"

struct idt_gate {
    uint16_t offset_low;
    uint16_t segment;
    uint16_t flags;
    uint16_t offset_high;
};

struct idtr_reg {
    uint8_t push_it_to_the[sizeof(uintptr_t)-sizeof(uint16_t)];
    uint16_t limit;
    uint32_t base;
};

struct idt_gate idt[32];

void halt(void);
void init_idt(void);

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

end:
    kprintf("End of loader_main.\n");
    halt();
}

#define MAP_INTERRUPT(vector, address) \
    do { \
        idt[vector].offset_high = (uint16_t)(((uint32_t)address >> 16) & 0xFFFF); \
        idt[vector].offset_low = (uint16_t)((uint32_t)address & 0xFFFF); \
        idt[vector].segment = 0x8; \
        idt[vector].flags = (1 << 15) | /* segment present */ \
                                   /* Descriptor Privilege level = 0 */ \
                       (1 << 11) | /* Descpiptor size = 32 bit */ \
                       (1 << 10) | /* always 1 */ \
                       (1 << 9); /* always 1 */ \
    } while (0);

void
init_idt(void)
{
    MAP_INTERRUPT(0, interrupt0);
    MAP_INTERRUPT(1, interrupt1);
    MAP_INTERRUPT(2, interrupt2);
    MAP_INTERRUPT(3, interrupt3);
    MAP_INTERRUPT(4, interrupt4);
    MAP_INTERRUPT(5, interrupt5);
    MAP_INTERRUPT(6, interrupt6);
    MAP_INTERRUPT(7, interrupt7);
    MAP_INTERRUPT(8, interrupt8);
    MAP_INTERRUPT(9, interrupt9);
    MAP_INTERRUPT(10, interrupt10);
    MAP_INTERRUPT(11, interrupt11);
    MAP_INTERRUPT(12, interrupt12);
    MAP_INTERRUPT(13, interrupt13);
    MAP_INTERRUPT(14, interrupt14);
    MAP_INTERRUPT(15, interrupt15);
    MAP_INTERRUPT(16, interrupt16);
    MAP_INTERRUPT(17, interrupt17);
    MAP_INTERRUPT(18, interrupt18);
    MAP_INTERRUPT(19, interrupt19);
    MAP_INTERRUPT(20, interrupt20);
    MAP_INTERRUPT(21, interrupt21);
    MAP_INTERRUPT(22, interrupt22);
    MAP_INTERRUPT(23, interrupt23);
    MAP_INTERRUPT(24, interrupt24);
    MAP_INTERRUPT(25, interrupt25);
    MAP_INTERRUPT(26, interrupt26);
    MAP_INTERRUPT(27, interrupt27);
    MAP_INTERRUPT(28, interrupt28);
    MAP_INTERRUPT(29, interrupt29);
    MAP_INTERRUPT(30, interrupt30);
    MAP_INTERRUPT(31, interrupt31);

    struct idtr_reg reg;
    reg.limit = sizeof(struct idt_gate) * 32;
    reg.base = (uint32_t)idt;

    __asm__ volatile ("lidt %0" :: "m"(reg.limit));
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
