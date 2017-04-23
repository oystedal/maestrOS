#include "stdint-gcc.h"
#include "multiboot.h"
#include "../util.h"
#include "interrupts.h"
#include "memory.h"
#include "elf.h"

extern const char _loader_start[];
extern const char _loader_end[];

extern uint64_t kernel_pml4;
extern uint64_t kernel_pdpt;
extern uint64_t kernel_pd;
extern uint64_t kernel_pt;

unsigned long load_elf64_module(uint32_t address);
uint32_t load_elf64_kernel(uint32_t address);

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

void halt(void);
void parse_multiboot_info(const multiboot_info_t* info);
void load_kernel(const multiboot_info_t* info);

struct mem_regions;
extern struct mem_regions regions;

void
loader_main(const uint32_t magic,
            const uint32_t addr)
{
    // Load our own GDT.
    __asm__ volatile ("lgdt gdtr");

    // Reload segment selectors.
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

    // Set up a temporary interrupt vector for our loader.
    init_idt();

    // Clear the screen.
    cls();

    // Check that we're loaded by an actual multiboot-compliant loader.
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        kprintf("Incorrect multiboot magic.\n");
        kprintf("Expected 0x%x - Actual: 0x%x\n",
                MULTIBOOT_BOOTLOADER_MAGIC,
                magic);
        goto end;
    }

    early_init_memory();
    enable_paging();

    // Map the multiboot info to a virtual address, since it can be anywhere.
    const uint32_t multiboot_info_region = (uint32_t)map_region(addr, 0x1000);
    const multiboot_info_t *info = (void*)multiboot_info_region;

    // Read some information from the multiboot structure.
    // - Add useable memory regions.
    // - Remove memory regions occupied by modules.
    parse_multiboot_info(info);

    // Remove anything below 0x10000 for simplicity.
    memory_remove_region(0x0, 0x10000);

    // Remove the region occupied by this loader itself.
    memory_remove_region((uint32_t)_loader_start,
                         (uint32_t)_loader_end - (uint32_t)_loader_start);

    // Load the kernel module.
    load_kernel(info);

    // Remove the mapping of the virtual address to the multiboot structure.
    unmap_region(addr, 0x1000);

    // Disable paging
    __asm__ volatile (
            "movl %cr0, %eax;"
            "andl $~0x80000000, %eax;"
            "movl %eax, %cr0"
            );

    // Enable PAE
    __asm__ volatile (
            "movl %cr4, %eax;"
            "orl $0x20, %eax;"
            "movl %eax, %cr4;"
            );

    // Load PML4 address into cr3
    __asm__ volatile ("movl %0, %%cr3;" :: "c"(&kernel_pml4));

    // Enable long mode (64-bit)
    __asm__ volatile (
            "movl $0xC0000080, %ecx;"
            "movl $0, %edx;"
            "rdmsr;"
            "orl $256, %eax;"
            "wrmsr;"
            );

    // Enable paging
    __asm__ volatile (
            "movl %cr0, %eax;"
            "orl $0x80000000, %eax;"
            "movl %eax, %cr0"
            );

    // Load the new 64-bit gdt
    __asm__ volatile ("lgdt gdtr64");

    // This needs to be volatile to not be moved to the other side of the ljmp.
    uint64_t volatile kaddr = kernel_entry;

    // Far jump to fully enter long mode and load a new CS.
    // Reload all other segments.
    __asm__ volatile (
            "ljmp $0x8, $long_mode_world;"
            "long_mode_world:"
            "mov $0x10, %eax;"
            "mov %eax, %ds;"
            "mov %eax, %es;"
            "mov %eax, %fs;"
            "mov %eax, %gs;"
            "mov %eax, %ss;"
            );

    // Provide the memory map as the first parameter to the kernel.
    __asm__ volatile ("movl %0, %%edi" :: "c"(&regions));

    // Jump to the kernel
    __asm__ volatile ("jmp *(%0)" :: "r"(&kaddr));

end:
    kprintf("End of loader_main.\n");
    halt();
}

void
parse_multiboot_info(const multiboot_info_t *info)
{
    struct mmap_entry *mmap = (void*)map_region(info->mmap_addr, 0x1000);

    for (uint32_t mmap_end = (uint32_t)mmap + info->mmap_length; (uint32_t)mmap < mmap_end; mmap = (struct mmap_entry *)((uint32_t)mmap + mmap->size + sizeof(mmap->size))) {
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


    multiboot_module_t *mod = (void*)map_region(info->mods_addr, 0x1000);
    for (uint32_t i = 0; i < info->mods_count; i++) {
#if 0
        kprintf("mod %d - start: 0x%x end: 0x%x\n", i, mod->mod_start, mod->mod_end);
#endif
        memory_remove_region(mod->mod_start, mod->mod_end);
        mod++;
    }
}

#define ptr_to_uint64(x) ((uint64_t)(uint32_t)x)

void
load_kernel(const multiboot_info_t *info)
{
    uint64_t* pml4 = &kernel_pml4;
    uint64_t* pdp  = &kernel_pdpt;
    uint64_t* pd   = &kernel_pd;
    uint64_t* pt   = &kernel_pt;

    const uint64_t rw_flags = (1 << 1) | 1; // P + R/W
    const uint64_t ro_flags = 1; // P + R/O

    *(pml4) = ptr_to_uint64(pdp) | rw_flags;
    *(pdp)  = ptr_to_uint64(pd)  | rw_flags;
    *(pd)   = ptr_to_uint64(pt)  | rw_flags;

    // Loader
    for (uint64_t i = (uint32_t)_loader_start / 0x1000; i < (uint32_t)_loader_end / 0x1000; ++i) {
        *(pt + i) = i << 12 | rw_flags;
    }

    // Video
    pt[0xB8000 >> 12] = 0xB8000 | rw_flags;

    multiboot_module_t *mod = (void*)map_region(info->mods_addr, 0x1000);
    uint32_t m = (uint32_t)map_region(mod->mod_start, 0x1000);

    unsigned char *mod_addr = (unsigned char*)m;

    if (*(mod_addr) != 0x7f &&
        *(mod_addr+1) != 'E' &&
        *(mod_addr+2) != 'L' &&
        *(mod_addr+3) != 'F')
    {
        kprintf("Kernel does not appear to be an ELF binary.\n");
        halt();
    }

    cls();
    kprintf("Loading kernel...\n");

    Elf64_Ehdr *elf = (Elf64_Ehdr*)mod_addr;
    Elf64_Phdr *phdr = (Elf64_Phdr*)(mod_addr + elf->e_phoff);

    for (unsigned int ph = 0; ph < elf->e_phnum; ++ph) {
        uint32_t vaddr = phdr->p_vaddr;

        uint32_t file_offset = phdr->p_offset;
        while ((uint32_t)vaddr < phdr->p_vaddr + phdr->p_memsz) {
            uint32_t page = (uint32_t)alloc_page_frame();

            memzero((void*)page, 0x1000);
            if (phdr->p_filesz)
                memcpy((void*)page, mod_addr + file_offset, 0x1000);

            if (phdr->p_flags & (PF_W)) {
                pt[vaddr >> 12] = (uint64_t)page | rw_flags;
            } else {
                pt[vaddr >> 12] = (uint64_t)page | ro_flags;
            }

            vaddr += 0x1000;
            file_offset += 0x1000;
        }

        phdr++;
    }

    kprintf("Kernel entry: 0x%x\n", (uint32_t)elf->e_entry);
    kprintf("Done.\n", (uint32_t)elf->e_entry);

    kernel_entry = (uint32_t)elf->e_entry;

    unmap_region(info->mods_addr, 0x1000);
}

void
halt(void)
{
    kprintf("Halting.\n");
    BXBREAK;
    for (;;) {
        __asm__ volatile ("cli");
        __asm__ volatile ("hlt");
    }
}
