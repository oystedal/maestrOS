#ifndef MEMORY_H_INCLUDE
#define MEMORY_H_INCLUDE

#include <stdint-gcc.h>

void early_init_memory(void);
void enable_paging(void);

void* map_region(uint32_t address, uint32_t limit);
void unmap_region(uint32_t address, uint32_t limit);
void memory_add_region(uint32_t base, uint32_t length);
void memory_remove_region(uint32_t base, uint32_t length);

#endif /* ifndef MEMORY_H_INCLUDE */
