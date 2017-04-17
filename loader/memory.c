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
struct mem_region region[32];
uint32_t num_regions = 0;

static void
swap_regions(int i, int j)
{
    struct mem_region tmp;
    tmp.length = region[i].length;
    tmp.base = region[i].base;

    region[i].base = region[j].base;
    region[i].length = region[j].length;

    region[j].base = tmp.base;
    region[j].length = tmp.length;

}

static void
sort_regions(void)
{
    for (uint32_t i = 0; i < num_regions; ++i) {
        for (uint32_t j = i + 1; j < num_regions; j++) {
            if (region[i].base > region[j].base) {
                swap_regions(i,j);
            }
        }
    }
}

static void
merge_regions(void)
{
    for (uint32_t i = 0; i < num_regions - 1; ++i) {
redo:
        if (region[i].base + region[i].length >= region[i + 1].base) {
            region[i].length = region[i + 1].base - region[i].base + region[i + 1].length;
            swap_regions(i + 1, num_regions - 1);
            num_regions--;
            goto redo;
        }
    }
}

static void
sanitize_regions(void)
{
    sort_regions();
    merge_regions();

    for (uint32_t i = 0; i < num_regions; ++i) {
        kprintf("Region %d base: 0x%x length 0x%x\n", i, region[i].base, region[i].length);
    }
}

void
memory_add_region(uint32_t base, uint32_t length)
{
    kprintf("%s(0x%x, 0x%x)\n", __func__, base, length);

    if ((base & 0xFFF) != 0) {
        uint32_t diff = ((base + 0x1000) & ~0xFFF) - base;
        length -= diff;
        base += diff;
    }

    length &= ~0xFFF;
    if (length == 0)
        return;

    region[num_regions].base = base;
    region[num_regions].length = length;
    region[num_regions].valid = 1;
    num_regions++;

    sanitize_regions();
}

void
memory_remove_region(uint32_t base, uint32_t length)
{
    base &= ~0xFFF;
    if (length & 0xFFF) {
        length = (length & ~0xFFF) + 0x1000;
    }

    kprintf("%s(0x%x, 0x%x)\n", __func__, base, length);

restart:
    for (uint32_t i = 0; i < num_regions; ++i) {
        const uint32_t end = base + length;

        // Case 1: Simple
        if (base <= region[i].base && end >= region[i].base + region[i].length) {
            kprintf("Case 1\n");
            swap_regions(i, num_regions - 1);
            num_regions--;
            goto restart;
        }

        // Case 2: Removed region extends into the lower part of this region
        if (base <= region[i].base && end <= region[i].base + region[i].length) {
            kprintf("Case 2\n");
            uint32_t new_base = base + length;
            region[i].length -= new_base - base;
            region[i].base = new_base;
            break;
        }

        // Case 3: Base is above this region
        if (base > region[i].base + region[i].length)
            continue;

        // Case 4: Removed region is completely inside this region
        if (base > region[i].base && end < region[i].base + region[i].length) {

            uint32_t new_region = num_regions++;
            region[new_region].valid = 1;
            region[new_region].base = end;
            region[new_region].length = region[i].base + region[i].length - (end);
            // region[i].base is unchanged
            region[i].length = base - region[i].base;
            break;
        }

        kprintf("Region %d base: 0x%x length 0x%x\n", i, region[i].base, region[i].length);
        kprintf("Unhandled case\n");
        BXBREAK;
    }

    sanitize_regions();
}

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
