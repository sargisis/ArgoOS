// Task Management Implementation

#include "task.h"
#include "pmm.h"
#include "paging.h"
#include "heap.h"
#include "string.h"
#include "vga.h"

struct task* task_list = NULL;
struct task* current_task = NULL;
uint32_t next_task_id = 1;
uint32_t task_count = 0;

// Default time slice (in timer ticks)
#define DEFAULT_TIME_SLICE 10

// Create kernel stack for task
uint32_t create_kernel_stack(void) {
    // Allocate 4KB for kernel stack
    void* stack_page = pmm_alloc_page();
    if (stack_page == NULL) {
        return 0;
    }
    
    // Map stack page to virtual address 0xE0000000 + (task_id * 0x1000)
    uint32_t virtual_addr = 0xE0000000 + (next_task_id * 0x1000);
    paging_map_page(virtual_addr, (uint32_t)stack_page, PAGE_PRESENT | PAGE_WRITABLE);
    
    // Stack grows downward, so return top of stack
    return virtual_addr + 4096;
}

void task_init(void) {
    task_list = NULL;
    current_task = NULL;
    next_task_id = 1;
    task_count = 0;
    
    // Create idle task (runs when no other tasks are ready)
    // For now, we'll just initialize the task manager
}

struct task* task_create(void (*entry)(void), const char* name, uint32_t priority) {
    if (task_count >= MAX_TASKS) {
        return NULL; // Too many tasks
    }
    
    // Allocate task structure
    struct task* new_task = (struct task*)kmalloc(sizeof(struct task));
    if (new_task == NULL) {
        return NULL;
    }
    
    // Initialize task
    new_task->id = next_task_id++;
    new_task->state = TASK_READY;
    new_task->priority = priority;
    new_task->time_slice = DEFAULT_TIME_SLICE;
    new_task->page_directory = 0; // Use kernel page directory for now
    new_task->next = NULL;
    
    if (name != NULL) {
        strncpy(new_task->name, name, 31);
        new_task->name[31] = '\0';
    } else {
        strcpy(new_task->name, "task");
    }
    
    // Create kernel stack
    uint32_t stack_top = create_kernel_stack();
    if (stack_top == 0) {
        kfree(new_task);
        return NULL;
    }
    
    // Set up initial stack frame
    // Stack layout for initial context:
    // [esp+44] eflags
    // [esp+40] cs
    // [esp+36] eip (entry point)
    // [esp+32] eax
    // [esp+28] ecx
    // [esp+24] edx
    // [esp+20] ebx
    // [esp+16] esp (dummy)
    // [esp+12] ebp
    // [esp+8]  esi
    // [esp+4]  edi
    // [esp+0]  (top of stack)
    
    uint32_t* stack = (uint32_t*)(stack_top - sizeof(struct task_context));
    
    // Set up initial context
    stack[0] = 0; // edi
    stack[1] = 0; // esi
    stack[2] = 0; // ebp
    stack[3] = (uint32_t)stack; // esp
    stack[4] = 0; // ebx
    stack[5] = 0; // edx
    stack[6] = 0; // ecx
    stack[7] = 0; // eax
    stack[8] = (uint32_t)entry; // eip
    stack[9] = 0x08; // cs (kernel code segment)
    stack[10] = 0x200; // eflags (interrupts enabled)
    
    new_task->esp = (uint32_t)stack;
    new_task->ebp = (uint32_t)stack;
    new_task->eip = (uint32_t)entry;
    
    // Add to task list
    if (task_list == NULL) {
        task_list = new_task;
        new_task->next = new_task; // Circular list
    } else {
        new_task->next = task_list->next;
        task_list->next = new_task;
        task_list = new_task;
    }
    
    task_count++;
    
    return new_task;
}

// Assembly function to switch context
extern void task_switch_asm(uint32_t* old_esp, uint32_t new_esp);

void task_switch(void) {
    if (task_list == NULL) {
        return; // No tasks to switch to
    }
    
    // Find next ready task
    struct task* next = current_task;
    if (next == NULL) {
        next = task_list;
    } else {
        next = next->next;
    }
    
    // Round-robin: find next ready task
    uint32_t attempts = 0;
    while (next->state != TASK_READY && attempts < task_count) {
        next = next->next;
        attempts++;
    }
    
    if (next->state != TASK_READY) {
        return; // No ready tasks
    }
    
    // Save current task's stack pointer
    uint32_t old_esp;
    asm volatile("mov %%esp, %0" : "=r"(old_esp));
    
    if (current_task != NULL) {
        current_task->esp = old_esp;
        if (current_task->state == TASK_RUNNING) {
            current_task->state = TASK_READY;
        }
    }
    
    // Switch to next task
    current_task = next;
    current_task->state = TASK_RUNNING;
    current_task->time_slice = DEFAULT_TIME_SLICE;
    
    // Load new task's stack pointer and switch
    uint32_t new_esp = current_task->esp;
    task_switch_asm(&old_esp, new_esp);
}

struct task* task_get_current(void) {
    return current_task;
}

void task_yield(void) {
    // Trigger context switch
    task_switch();
}

void task_exit(void) {
    if (current_task == NULL) {
        return;
    }
    
    current_task->state = TASK_ZOMBIE;
    
    // Remove from task list (simplified - just mark as zombie)
    // In a real OS, we'd need to clean up resources
    
    // Switch to next task
    task_switch();
}

void task_sleep(uint32_t milliseconds) {
    (void)milliseconds; // TODO: Implement proper sleep using timer
    if (current_task == NULL) {
        return;
    }
    
    // For now, just yield (proper implementation would use timer)
    current_task->state = TASK_BLOCKED;
    task_switch();
}

void task_wake(struct task* task) {
    if (task != NULL && task->state == TASK_BLOCKED) {
        task->state = TASK_READY;
    }
}

struct task* task_get_by_id(uint32_t id) {
    if (task_list == NULL) {
        return NULL;
    }
    
    struct task* current = task_list;
    do {
        if (current->id == id) {
            return current;
        }
        current = current->next;
    } while (current != task_list);
    
    return NULL;
}
