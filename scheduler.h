#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "stdint.h"

typedef struct tcb_s {
    uint64_t stack_address;
    uint64_t initialized;
    uint64_t start_address;
} tcb_t;

void yield(void);

void scheduler_init(void);
void scheduler_add(uint64_t start_addr, uint64_t stack_addr);
void scheduler_start(void);

#endif /* SCHEDULER_H */
