#include "interrupts.h"

struct idt_gate {
    uint16_t offset_low;
    uint16_t segment;
    uint16_t flags;
    uint16_t offset_high;
};

struct idtr_reg {
    char push_it_to_the[sizeof(uintptr_t)-sizeof(uint16_t)];
    uint16_t limit;
    uint32_t base;
};

struct idt_gate idt[32];

void
init_idt(void)
{
    _Static_assert(sizeof(struct idt_gate) == 8, "IDT gate structure is of incorrect size");
    // _Static_assert(sizeof(struct idtr_reg) == 8, "IDTR register structure is of incorrect size");

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

    __asm__ volatile ("lidt (%[idtr])" :: [idtr] "r" (&reg.limit) : "memory");
}

