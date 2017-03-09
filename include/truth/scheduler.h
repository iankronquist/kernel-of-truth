#pragma once

enum status scheduler_init(void);
enum status scheduler_fini(void);
void scheduler_yield(void);
struct process *scheduler_current_process(void);
enum status scheduler_add_process(struct process *proc);
enum status scheduler_remove_process(struct process *proc);
