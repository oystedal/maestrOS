#include "stdint-gcc.h"
#include "multiboot.h"
#include "../util.h"
#include "interrupts.h"
#include "memory.h"

struct mmap_entry
{
    uint32_t size;
    uint32_t addr_high;
    uint32_t addr_low;
    uint32_t len_high;
    uint32_t len_low;
    uint32_t type;
};

void halt(void);
void parse_multiboot_info(uint32_t addr);

void loader_main(uint32_t magic, uint32_t addr)
{
    (void)addr;

    __asm__ volatile ("lgdt gdtr");
    __asm__ volatile (
            "ljmp $0x8, $reload_cs;"
            "reload_cs:"
            "mov $0x10, %eax;"
            "mov %eax, %ds;"
            "mov %eax, %es;"
            "mov %eax, %fs;"
            "mov %eax, %gs;"
            "mov %eax, %ss;"
    );

    init_idt();

    // clear screen
    cls();

    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        kprintf("Incorrect multiboot magic.\n");
        kprintf("Expected 0x%x - Actual: 0x%x\n",
                MULTIBOOT_BOOTLOADER_MAGIC,
                magic);
        goto end;
    }

    kprintf("Multiboot magic OK\n");

    early_init_memory();
    enable_paging();

    parse_multiboot_info(addr);

end:
    kprintf("End of loader_main.\n");
    halt();
}

void
parse_multiboot_info(uint32_t addr)
{
    uint32_t region = (uint32_t)map_region(addr, 0x1000);
    multiboot_info_t *info = (void*)region;
    kprintf("mmap addr: 0x%x\n", info->mmap_addr);
    kprintf("mmap length: 0x%x\n", info->mmap_length);

    struct mmap_entry *mmap = (void*)map_region(info->mmap_addr, 0x1000);
    uint32_t mmap_end = (uint32_t)mmap + info->mmap_length;

    for (; (uint32_t)mmap < mmap_end; mmap = (struct mmap_entry *)((uint32_t)mmap + mmap->size + sizeof(mmap->size))) {
        kprintf("size = 0x%x, base_addr = 0x%x:%x, length = 0x%x:%x, type = %s\n",
                mmap->size,
                mmap->addr_high,
                mmap->addr_low,
                mmap->len_high,
                mmap->len_low,
                mmap->type == MULTIBOOT_MEMORY_AVAILABLE ? "available" : "reserved");
    }
}

void halt(void)
{
    kprintf("Halting.\n");
    BXBREAK;
    for (;;) {
        __asm__ volatile ("cli");
        __asm__ volatile ("hlt");
    }
}
