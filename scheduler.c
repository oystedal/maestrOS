#include "scheduler.h"
#include "util.h"

tcb_t* current_task;
uint64_t current_task_idx;

static uint64_t task_count;
static tcb_t tasks[10];

void dispatch(void);

void scheduler_init(void)
{
    task_count = 0;
    current_task = (void*)0;
    current_task_idx = 0;
}

void scheduler_add(uint64_t start_addr, uint64_t stack_addr)
{
    tasks[task_count].start_address = start_addr;
    tasks[task_count].stack_address = stack_addr;
    tasks[task_count].initialized = 0;
    task_count++;
}

void scheduler_start(void)
{
    // BXBREAK;

    current_task = &tasks[0];

    __asm__ volatile ("mov %0, %%rsp" :: "c"(current_task->stack_address));

    if (!current_task->initialized) {
        __asm__ volatile ("pushq %0" :: "c"(current_task->start_address));
    }

    __asm__ volatile ("jmp dispatch");
}

void schedule(void)
{
    current_task_idx = (current_task_idx + 1) % task_count;
    current_task = &tasks[current_task_idx];

    __asm__ volatile ("mov %0, %%rsp" :: "c"(current_task->stack_address));

    if (!current_task->initialized) {
        __asm__ volatile ("pushq %0" :: "c"(current_task->start_address));
    }

    __asm__ volatile ("jmp dispatch");
}
