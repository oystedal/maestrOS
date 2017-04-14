#ifndef MEMORY_H_INCLUDE
#define MEMORY_H_INCLUDE

#include <stdint-gcc.h>

void early_init_memory(void);
void enable_paging(void);

void* map_region(uint32_t address, uint32_t limit);

#endif /* ifndef MEMORY_H_INCLUDE */
