// System Calls
// Interface for user-space programs to request kernel services

#ifndef SYSCALL_H
#define SYSCALL_H

#include "types.h"

// System call numbers
#define SYS_EXIT    0
#define SYS_YIELD   1
#define SYS_SLEEP   2
#define SYS_WRITE   3
#define SYS_READ    4
#define SYS_FORK    5
#define SYS_EXEC    6

// System call handler type
typedef int32_t (*syscall_handler_t)(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);

// Initialize system calls
void syscall_init(void);

// Handle system call
int32_t syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);

// System call stubs (called from user space)
extern void syscall_entry(void);

#endif // SYSCALL_H
