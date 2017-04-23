#include "util.h"
#include "stdint.h"

struct mem_region {
    uint32_t valid;
    uint32_t base;
    uint32_t length;
};

struct mem_regions {
    uint32_t count;
    struct mem_region regions[1];
};

static const char* fact = "Today is tomorrow's yesterday.";

uint32_t
fib(uint32_t n)
{
    if (n == 0) return 0;
    if (n == 1) return 1;
    return n + fib(n-1);
}

void _start(uint32_t meaning_of_life)
{
    __asm__ volatile ("andq $0xFFFFFFFFFFFFFFFF, %rsp");

    cls();
    kprintf("The kernel has discovered the following facts:\n");
    kprintf("  2+2 = %d\n", 2+2);
    kprintf("  fib(10) = %d\n", fib(10));
    kprintf("  meaning_of_life = %d\n", meaning_of_life);
    kprintf("  %s\n", fact);
    kprintf("\n");
    kprintf("Please stand by for more interesting facts as they are discovered...\n");

    struct mem_regions *regions = (void*)meaning_of_life;
    for (uint32_t i = 0; i < regions->count; ++i) {
        struct mem_region *region = &(regions->regions[i]);
        kprintf("Region %d: base: 0x%x length 0x%x\n", i, region->base, region->length);
    }

    for (;;) {
        __asm__ volatile ("cli");
        __asm__ volatile ("hlt");
    }
}
