#pragma once

#include <truth/types.h>

#define Syscall_Max 0x100

typedef uint64_t (*syscall_f)();

enum status syscall_init(void);
void syscall_fini(void);
enum status syscall_register(uint64_t syscall_number, syscall_f function);
void syscall_unregister(uint64_t syscall_number);
