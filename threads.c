#include "scheduler.h"
#include "util.h"

void thread1(void)
{
    int a = 1; int b = 2; int c = 3; int d = 4; int e = 5; int f = 6;
    kprintf("Thread 1 - Start\n");
    while (1) {
        kprintf("Thread 1 - %d %d %d %d %d %d\n", a, b, c, d, e, f);
        BXBREAK;
        yield();
        BXBREAK;
        if (a + 1 == b && b + 1 == c && c + 1 == d && d + 1 == e && e + 1 == f) {
            a++; b++; c++; d++; e++; f++;
        } else {
            while (1);
        }
    }
}

void thread2(void)
{
    int a = 2; int b = 2; int c = 2; int d = 2; int e = 2; int f = 2;
    kprintf("Thread 2 - Start\n");
    while (1) {
        kprintf("Thread 2 - %d %d %d %d %d %d\n", a, b, c, d, e, f);
        yield();
        if (a == b && b == c && c == d && d == e && e == f) {
            a++; b++; c++; d++; e++; f++;
        } else {
            while (1);
        }
    }
}

void thread3(void)
{
    int a = 3; int b = 3; int c = 3; int d = 3; int e = 3; int f = 3;
    kprintf("Thread 3 - Start\n");
    while (1) {
        kprintf("Thread 3 - %d %d %d %d %d %d\n", a, b, c, d, e, f);
        yield();
        if (a == b && b == c && c == d && d == e && e == f) {
            a++; b++; c++; d++; e++; f++;
        } else {
            while (1);
        }
    }
}

void thread4(void)
{
    int a = 4; int b = 4; int c = 4; int d = 4; int e = 4; int f = 4;
    kprintf("Thread 4 - Start\n");
    while (1) {
        kprintf("Thread 4 - %d %d %d %d %d %d\n", a, b, c, d, e, f);
        yield();
        if (a == b && b == c && c == d && d == e && e == f) {
            a++; b++; c++; d++; e++; f++;
        } else {
            while (1);
        }
    }
}
