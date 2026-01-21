// System Calls Implementation

#include "syscall.h"
#include "task.h"
#include "vga.h"
#include "serial.h"
#include "timer.h"
#include "fs.h"
#include "string.h"
#include "heap.h"
#include "elf.h"

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
    (void)arg4;
    if (!buf || count == 0) {
        return -1;
    }
    
    char* buffer = (char*)buf;
    
    // Standard file descriptors:
    // 0 = stdin, 1 = stdout, 2 = stderr
    if (fd == 1 || fd == 2) {
        // Write to stdout/stderr (serial port for now)
        for (uint32_t i = 0; i < count; i++) {
            serial_putchar(buffer[i]);
        }
        return count;
    } else if (fd >= 3) {
        // Write to file
        int32_t written = fs_write(fd, buffer, count);
        return written;
    }
    
    return -1; // Invalid file descriptor
}

static int32_t sys_read(uint32_t fd, uint32_t buf, uint32_t count, uint32_t arg4) {
    (void)arg4;
    if (!buf || count == 0) {
        return -1;
    }
    
    char* buffer = (char*)buf;
    
    // Standard file descriptors:
    // 0 = stdin, 1 = stdout, 2 = stderr
    if (fd == 0) {
        // Read from stdin (serial port)
        uint32_t bytes_read = 0;
        
        // Read characters until we have count bytes or newline
        while (bytes_read < count) {
            // Wait for data to be available
            while (!serial_is_data_available()) {
                // Yield to other tasks while waiting
                asm volatile("hlt");
            }
            
            char c = serial_getchar();
            if (c == 0) {
                continue; // No data available
            }
            
            buffer[bytes_read++] = c;
            
            // Stop on newline (for line-based input)
            if (c == '\n' || c == '\r') {
                break;
            }
        }
        
        return bytes_read;
    } else if (fd >= 3) {
        // Read from file
        int32_t bytes_read = fs_read(fd, buffer, count);
        return bytes_read;
    }
    
    return -1; // Invalid file descriptor
}

static int32_t sys_fork(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg1; (void)arg2; (void)arg3; (void)arg4;
    
    struct task* parent = task_get_current();
    if (parent == NULL) {
        return -1; // No current task
    }
    
    // Check if we can create more tasks
    extern uint32_t task_count;
    if (task_count >= 256) { // MAX_TASKS
        return -1; // Too many tasks
    }
    
    // Allocate new task structure
    struct task* child = (struct task*)kmalloc(sizeof(struct task));
    if (child == NULL) {
        return -1; // Out of memory
    }
    
    // Copy parent's task structure
    memcpy(child, parent, sizeof(struct task));
    
    // Assign new task ID
    extern uint32_t next_task_id;
    child->id = next_task_id++;
    child->state = TASK_READY;
    
    // Create new kernel stack for child
    extern uint32_t create_kernel_stack(void);
    uint32_t child_stack_top = create_kernel_stack();
    if (child_stack_top == 0) {
        kfree(child);
        return -1; // Failed to create stack
    }
    
    // Copy parent's stack to child's stack
    // We need to copy the entire stack frame
    uint32_t parent_stack = parent->esp;
    uint32_t* parent_stack_ptr = (uint32_t*)parent_stack;
    uint32_t* child_stack_ptr = (uint32_t*)(child_stack_top - sizeof(struct task_context));
    
    // Copy stack context (11 words: edi, esi, ebp, esp, ebx, edx, ecx, eax, eip, cs, eflags)
    for (int i = 0; i < 11; i++) {
        child_stack_ptr[i] = parent_stack_ptr[i];
    }
    
    // Modify child's stack to return 0 (fork returns 0 in child)
    // The return value is in eax (stack[7])
    child_stack_ptr[7] = 0; // eax = 0 in child
    
    // Set child's stack pointer
    child->esp = (uint32_t)child_stack_ptr;
    
    // Add child to task list
    extern struct task* task_list;
    if (task_list == NULL) {
        task_list = child;
        child->next = child; // Circular list
    } else {
        child->next = task_list->next;
        task_list->next = child;
    }
    
    // Increment task count
    task_count++;
    
    // Return child's PID in parent (this will be the return value)
    return (int32_t)child->id;
}

static int32_t sys_exec(uint32_t path_ptr, uint32_t arg2, uint32_t arg3, uint32_t arg4) {
    (void)arg2; (void)arg3; (void)arg4;
    
    if (!path_ptr) {
        return -1; // Invalid path
    }
    
    char* path = (char*)path_ptr;
    
    // Load ELF executable
    int entry_point = elf_load(path);
    if (entry_point < 0) {
        serial_puts("sys_exec: Failed to load executable\n");
        return -1;
    }
    
    // Get current task
    struct task* current = task_get_current();
    if (!current) {
        return -1;
    }
    
    // Set new entry point
    current->eip = entry_point;
    
    // Reset stack pointer (simplified - in real OS we'd set up user stack)
    // For now, we'll keep the existing stack
    
    // The task will continue execution at the new entry point
    // when it's scheduled next
    
    serial_puts("sys_exec: Program loaded successfully\n");
    
    // Note: In a real exec, we'd replace the entire process image
    // For now, this is a simplified version
    
    return 0; // Success
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
