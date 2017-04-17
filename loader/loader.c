#include "stdint-gcc.h"
#include "multiboot.h"
#include "../util.h"
#include "interrupts.h"
#include "memory.h"
#include "elf.h"

extern const char _loader_start[];
extern const char _loader_end[];

unsigned long load_elf64_module(uint32_t address);

struct mmap_entry
{
    uint32_t size;
    uint32_t addr_low;
    uint32_t addr_high;
    uint32_t len_low;
    uint32_t len_high;
    uint32_t type;
};

uint64_t kernel_entry;
uint32_t* pml4;

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

    __asm__ volatile ("movl %cr0, %eax; andl $~0x80000000, %eax; movl %eax, %cr0");

    // Enable PAE
    __asm__ volatile ("movl %cr4, %eax; orl $0x20, %eax; movl %eax, %cr4");

    // Load PML4 address into cr3
    __asm__ volatile ("movl %0, %%cr3;" :: "c"(pml4) );

    // Enable 64-bit
    __asm__ volatile ("movl $0xC0000080, %ecx");
    __asm__ volatile ("movl $0, %edx");
    __asm__ volatile ("rdmsr");
    __asm__ volatile ("orl $256, %eax");
    __asm__ volatile ("wrmsr");

    // Enable paging
    __asm__ volatile ("movl %cr0, %eax; orl $0x80000000, %eax; movl %eax, %cr0");

    __asm__ volatile ("lgdt gdtr64");

    // This needs to be volatile to not be moved to the other side of the ljmp.
    uint64_t volatile kaddr = kernel_entry;

    __asm__ volatile (
            "ljmp $0x8, $long_mode_world; \n"
            "long_mode_world: \n"
            "mov $0x10, %eax;"
            "mov %eax, %ds; \n"
            "mov %eax, %es; \n"
            "mov %eax, %fs; \n"
            "mov %eax, %gs; \n"
            "mov %eax, %ss; \n"
            );

    BXBREAK;

    __asm__ volatile ("jmp %0" :: "r"(kaddr));

end:
    kprintf("End of loader_main.\n");
    halt();
}

void
parse_multiboot_info(uint32_t addr)
{
    uint32_t region = (uint32_t)map_region(addr, 0x1000);
    multiboot_info_t *info = (void*)region;
#if 0
    kprintf("mmap addr: 0x%x\n", info->mmap_addr);
    kprintf("mmap length: 0x%x\n", info->mmap_length);
#endif

    {
        struct mmap_entry *mmap = (void*)map_region(info->mmap_addr, 0x1000);
        uint32_t mmap_end = (uint32_t)mmap + info->mmap_length;

        for (; (uint32_t)mmap < mmap_end; mmap = (struct mmap_entry *)((uint32_t)mmap + mmap->size + sizeof(mmap->size))) {
#if 0
            kprintf("base = 0x%x:0x%x, length = 0x%x:0x%x, type = %s\n",
                    mmap->addr_high,
                    mmap->addr_low,
                    mmap->len_high,
                    mmap->len_low,
                    mmap->type == MULTIBOOT_MEMORY_AVAILABLE ? "available" : "reserved");
#endif

            if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
                if (mmap->addr_high == 0) {
                    if (mmap->len_high) {
                        kprintf("Ignoring region with len_high != 0\n");
                    } else {
                        memory_add_region(mmap->addr_low, mmap->len_low);
                    }
                } else {
                    // TODO: 64-bit memory pool
                }
            }
        }

        unmap_region(info->mmap_addr, 0x1000);
    }

#if 0
    kprintf("Loader start: 0x%x\n", (uint32_t)_loader_start);
    kprintf("Loader end: 0x%x\n", (uint32_t)_loader_end);
#endif
    memory_remove_region(0x0, 0x10000);
    memory_remove_region((uint32_t)_loader_start, (uint32_t)_loader_end - (uint32_t)_loader_start);

    {
#define ptr_to_uint64(x) ((uint64_t)(uint32_t)x)
        // uint32_t* code = alloc_page();
        // uint32_t* data = alloc_page();

        multiboot_module_t *mod = (void*)map_region(info->mods_addr, 0x1000);
        uint32_t count = info->mods_count;

        for (uint32_t i = 0; i < count; i++) {
            kprintf("mod %d - start: 0x%x end: 0x%x\n", i, mod->mod_start, mod->mod_end);
            memory_remove_region(mod->mod_start, mod->mod_end);

            pml4 = alloc_page();
            uint64_t* pdp = alloc_page();
            uint64_t* pd  = alloc_page();
            uint64_t* pt  = alloc_page();

            for (int i = 0; i < 512; ++i) {
                *(pml4+i) = 0;
                *(pdp+i)  = 0;
                *(pd+i)   = 0;
                *(pt+i)   = 0;
            }

            const uint64_t flags = (1 << 1) | 1; // P + R/W
            *(pml4) = ptr_to_uint64(pdp) | flags;
            *(pdp)  = ptr_to_uint64(pd)  | flags;
            *(pd)   = ptr_to_uint64(pt)  | flags;

            // Identity map 1MB
            for (uint64_t i = 0; i < 512; ++i) {
                *(pt + i) = i << 12 | flags;
            }

            uint32_t m = (uint32_t)map_region(mod->mod_start, 0x1000);
            kernel_entry = (uint64_t)load_elf64_module(m);

            mod++;
            break;
        }

        unmap_region(info->mods_addr, 0x1000);
    }

    unmap_region(addr, 0x1000);
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
