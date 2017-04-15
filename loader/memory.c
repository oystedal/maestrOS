#include "memory.h"
#include "../util.h"
#include <stdint-gcc.h>

extern uint32_t page_directory[1024];
extern uint32_t page_table[1024];

static inline void
map_directory_entry(uint32_t* table, uint32_t vaddr, uint32_t paddr)
{
    table[(vaddr >> 22) & 0x3FF] =
        (paddr & ~0xFFF) |
        (1 << 1) | // R/W
        (1 << 0) ; // Present
}

static inline void
map_table_entry(uint32_t* table, uint32_t vaddr, uint32_t paddr)
{
    table[(vaddr >> 12) & 0x3FF] =
        (paddr & ~0xFFF) |
        (1 << 1) | // R/W
        (1 << 0);  // Present
}

void
early_init_memory(void)
{
    for (uint32_t i = 0; i < 1024; ++i) {
        page_directory[i] = 0;
        page_table[i] = 0;
    }

    map_directory_entry(page_directory, 0x0, (uint32_t)&page_table);

    // Fractal(?) map
    page_directory[1023] = (uint32_t)page_directory;

    // ID map first few 4K pages sans 0 page
    // TODO: Find a nice way to limit this
    for (uint32_t i = 1; i < 25; ++i) {
        uint32_t addr = i * 0x1000;
        map_table_entry(page_table, addr, addr);
    }

    // ID map video memory
    map_table_entry(page_table, 0xB8000, 0xB8000);
}

void
enable_paging(void)
{
    // Load page directory
    __asm__ volatile ("movl %0, %%cr3" :: "a"(page_directory));

    // Enable paging
    __asm__ volatile (
            "movl %cr0, %eax;"
            "orl $0x80000000, %eax;"
            "movl %eax, %cr0;"
    );
    // BXBREAK;
}

void*
map_region(uint32_t address, uint32_t limit)
{
    kprintf("%s(0x%x, 0x%x)\n", __func__, address, limit);

    static uint32_t current_idx = 200;
    uint32_t start_address = address & ~0x3FF;
    uint32_t offset = address - (address & ~0x3FF);

    const uint32_t idx = current_idx;
    map_table_entry(page_table, idx * 0x1000, start_address);
    map_table_entry(page_table, (idx + 1) * 0x1000, start_address);
    current_idx++;

    return (void*)(idx * 0x1000 + offset);
}

void
unmap_region(uint32_t address, uint32_t limit)
{
    (void)address;
    (void)limit;
}

struct mem_region {
    uint32_t valid;
    uint32_t base;
    uint32_t length;
};

// TODO: Add more invariants to this list
// Sorted, non-overlapping regions, minimal representation.
static struct mem_region region[32];
static uint32_t num_regions = 0;

void
memory_add_region(uint32_t base, uint32_t length)
{
    if ((base & 0xFFF) != 0) {
        uint32_t diff = ((base + 0x1000) & ~0xFFF) - base;
        length -= diff;
        base += diff;
    }

    // TODO: Remove this dumb protection with something more substantial
    if (base < 0x20000) {
        if (length > 0x20000) {
            const uint32_t diff = 0x20000 - base;
            base += diff;
            length -= diff;
        } else {
            return;
        }
    }

    length &= ~0xFFF;
    if (length == 0)
        return;

    kprintf("Added region 0x%x - 0x%x\n", base, base + length);

    region[num_regions].base = base;
    region[num_regions].length = length;
    region[num_regions].valid = 1;
    num_regions++;
}

void halt(void);

void*
alloc_page(void)
{
    for (uint32_t i = 0; i < 32; ++i) {
        if (!region[i].valid)
            continue;

        if (region[i].length == 0)
            continue;

        uint32_t addr = region[i].base;
        region[i].base += 0x1000;
        region[i].length -= 0x1000;
        return (void*)addr;
    }

    kprintf("Out of memory\n");
    halt();

    return (void*)0;
}

void
memory_remove_region(uint32_t base, uint32_t length)
{
    (void)base;
    (void)length;
}
