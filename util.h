#ifndef UTIL_INCLUDE_H
#define UTIL_INCLUDE_H

#include "stdint.h"

#define BXBREAK \
    do { \
        __asm__ volatile ("xchg %bx, %bx"); \
    } while (0);

void cls(void);
void putchar(char c);
void kitoa(char* out, char base, long d);
void kprintf(const char* restrict format, ...);

void memcpy(void* dst, void* src, uint32_t size);
void memzero(void* dst, uint32_t count);

#endif // UTIL_INCLUDE_H
