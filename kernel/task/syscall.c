// System Calls Implementation

#include "syscall.h"
#include "task.h"
#include "vga.h"
#include "timer.h"

// System call handlers
static int32_t sys_exit(uint32_t exit_code, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg2; (void)arg3; (void)arg4;
    vga_puts("Task exited with code: ");
    vga_putdec(exit_code);
    vga_puts("\n");
    task_exit();
    return 0;
}

static int32_t sys_yield(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg1; (void)arg2; (void)arg3; (void)arg4;
    task_yield();
    return 0;
}

static int32_t sys_sleep(uint32_t milliseconds, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg2; (void)arg3; (void)arg4;
    task_sleep(milliseconds);
    return 0;
}

static int32_t sys_write(uint32_t fd, uint32_t buf, uint32_t count, uint32_t arg4) {
    (void)fd; (void)arg4;
    // Simple write to VGA (stdout)
    char* buffer = (char*)buf;
    for (uint32_t i = 0; i < count; i++) {
        vga_putchar(buffer[i]);
    }
    return count;
}

static int32_t sys_read(uint32_t fd, uint32_t buf, uint32_t count, uint32_t arg4) {
    (void)fd; (void)buf; (void)count; (void)arg4;
    // Not implemented yet
    return 0;
}

static int32_t sys_fork(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg1; (void)arg2; (void)arg3; (void)arg4;
    // Not implemented yet
    return -1;
}

static int32_t sys_exec(uint32_t path, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)path; (void)arg2; (void)arg3; (void)arg4;
    // Not implemented yet
    return -1;
}

// System call table
static syscall_handler_t syscall_table[] = {
    sys_exit,   // 0
    sys_yield,  // 1
    sys_sleep,  // 2
    sys_write,  // 3
    sys_read,   // 4
    sys_fork,   // 5
    sys_exec,   // 6
};

#define SYSCALL_COUNT (sizeof(syscall_table) / sizeof(syscall_handler_t))

void syscall_init(void) {
    // System calls will be initialized when we set up the IDT entry for INT 0x80
    // For now, just initialize the table
}

int32_t syscall_handler(uint32_t syscall_num, uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    if (syscall_num >= SYSCALL_COUNT) {
        return -1; // Invalid system call
    }
    
    return syscall_table[syscall_num](arg1, arg2, arg3, arg4);
}
