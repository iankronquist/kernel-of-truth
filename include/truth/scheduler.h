#pragma once

#include <truth/process.h>

enum status scheduler_init(struct process *);
void scheduler_fini(void);
void scheduler_yield(void);
struct thread *scheduler_get_current_thread(void);
struct process *scheduler_get_current_process(void);
void scheduler_add_thread(struct thread *thread);
void scheduler_remove_thread(struct thread *thread);
