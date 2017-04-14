#ifndef INTERRUPTS_INCLUDE_H
#define INTERRUPTS_INCLUDE_H

#include "stdint-gcc.h"

void init_idt(void);

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

void interrupt0(void);
void interrupt1(void);
void interrupt2(void);
void interrupt3(void);
void interrupt4(void);
void interrupt5(void);
void interrupt6(void);
void interrupt7(void);
void interrupt8(void);
void interrupt9(void);
void interrupt10(void);
void interrupt11(void);
void interrupt12(void);
void interrupt13(void);
void interrupt14(void);
void interrupt15(void);
void interrupt16(void);
void interrupt17(void);
void interrupt18(void);
void interrupt19(void);
void interrupt20(void);
void interrupt21(void);
void interrupt22(void);
void interrupt23(void);
void interrupt24(void);
void interrupt25(void);
void interrupt26(void);
void interrupt27(void);
void interrupt28(void);
void interrupt29(void);
void interrupt30(void);
void interrupt31(void);

#endif /* ifndef INTERRUPTS_INCLUDE_H */
