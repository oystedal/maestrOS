#ifndef UTIL_INCLUDE_H
#define UTIL_INCLUDE_H

#define BXBREAK \
    do { \
        __asm__ volatile ("xchg %bx, %bx"); \
    } while (0);

void cls(void);
void putchar(char c);
void kitoa(char* buf, int base, int d);
void kprintf(const char* restrict format, ...);

#endif // UTIL_INCLUDE_H
