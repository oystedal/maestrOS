#include <stdarg.h>
#include "util.h"
#include "stdint-gcc.h"

void
memcpy(void* dst, void* src, uint32_t size)
{
    char* _src = (char*)src;
    char* _dst = (char*)dst;

    for (uint32_t i = 0; i < size; ++i) {
        *(_dst + i) = *(_src + i);
    }
}

void
memzero(void* dst, uint32_t count)
{
    char* _dst = (char*)dst;
    kprintf("memzero(0x%x, 0x%x)\n", (uint32_t)_dst, count);

    for (uint32_t i = 0; i < count; ++i) {
        *(_dst + i) = 0;
    }
}

#define COLUMNS                 80
#define LINES                   24
#define ATTRIBUTE               7

unsigned char* const video = (unsigned char*)0xB8000;
uint32_t xpos = 0;
uint32_t ypos = 0;

void
cls(void)
{
    int i;

    for (i = 0; i < COLUMNS * LINES * 2; i++) {
        if (*(video + i) != 0)
            *(video + i) = 0;
    }

    xpos = 0;
    ypos = 0;
}

void
putchar(char c)
{
    if (c == '\n' || c == '\r')
    {
newline:
        xpos = 0;
        ypos++;
        if (ypos >= LINES)
            ypos = 0;
        return;
    }

    *(video + (xpos + ypos * COLUMNS) * 2) = c & 0xFF;
    *(video + (xpos + ypos * COLUMNS) * 2 + 1) = ATTRIBUTE;

    xpos++;
    if (xpos >= COLUMNS)
        goto newline;
}

void kitoa(char* out, char base, long d)
{
    char *p = out;
    char *p1, *p2;
    unsigned long ud = d;
    int divisor = 10;

    /* If %d is specified and D is minus, put `-' in the head. */
    if (base == 'd' && d < 0) {
        *p++ = '-';
        out++;
        ud = -d;
    } else if (base == 'x') {
        divisor = 16;
    }

    /* Divide UD by DIVISOR until UD == 0. */
    do {
        int r = ud % divisor;

        *p++ = (r < 10) ? r + '0' : r + 'a' - 10;
    }
    while (ud /= divisor);

    /* Terminate BUF. */
    *p = 0;

    /* Reverse BUF. */
    p1 = out;
    p2 = p - 1;
    while (p1 < p2)
    {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}

void
kprintf(const char* restrict format, ...)
{
    if (xpos == 0) {
        for (uint32_t i = 0; i < 80 * 2; i++) {
            *(video + (i + ypos * COLUMNS) * 2) = ' ' & 0xFF;
        }
    }

    char buf[20]; // TODO: Make some reasonable decision on the size
    va_list arg;
    va_start(arg, format);

    for (; format[0] != '\0'; format++) {
        if (format[0] != '%') {
            putchar(format[0]);
            continue;
        }

        format++;
        switch (*format) {
        case 's':
            {
                char* s = va_arg(arg, char*);
                while (*s) {
                    putchar(*s);
                    s++;
                }
                continue;
            }
        case 'd':
        case 'u':
        case 'x':
            {
                long n = va_arg(arg, int);
                kitoa(buf, *format, n);
                char* s = buf;
                while (*s) {
                    putchar(*s);
                    s++;
                }
                continue;
            }
       case 'c':
            {
                char c = (char)va_arg(arg, int);
                putchar(c);
            }
        }
    }

    va_end(arg);
    return;
}
