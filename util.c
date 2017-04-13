#include <stdarg.h>
#include "util.h"

#define COLUMNS                 80
#define LINES                   24
#define ATTRIBUTE               7

static unsigned char* const video = (unsigned char*)0xB8000;
static unsigned int xpos = 0;
static unsigned int ypos = 0;

void
cls(void)
{
    int i;

    for (i = 0; i < COLUMNS * LINES * 2; i++)
        *(video + i) = 0;

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
        case 's': {
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
        }
    }
    
    va_end(arg);
    return;
}
