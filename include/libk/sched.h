#ifndef SCHED_H
#define SCHED_H

// 30 ms in between wakeups
#define QUANTUM 30

struct process *process_table = NULL;
struct process *scheduling_queue = NULL;

void init_scheduling_queue(size_t initial_size);

void remove_from_queue(struct

void wakeup_handler(void);

#endif
