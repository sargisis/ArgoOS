// Task Management Implementation

#include "task.h"
#include "heap.h"
#include "paging.h"
#include "pmm.h"
#include "string.h"
#include "timer.h"
#include "vga.h"

struct task *task_list = NULL;
struct task *current_task = NULL;
uint32_t next_task_id = 1;
uint32_t task_count = 0;

// Default time slice (in timer ticks)
#define DEFAULT_TIME_SLICE 10

// Create kernel stack for task
uint32_t create_kernel_stack(void) {
  // Allocate 4KB for kernel stack
  void *stack_page = pmm_alloc_page();
  if (stack_page == NULL) {
    return 0;
  }

  // Map stack page to virtual address 0xE0000000 + (task_id * 0x1000)
  uint32_t virtual_addr = 0xE0000000 + (next_task_id * 0x1000);
  paging_map_page(virtual_addr, (uint32_t)stack_page,
                  PAGE_PRESENT | PAGE_WRITABLE);

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

struct task *task_create(void (*entry)(void), const char *name,
                         uint32_t priority) {
  if (task_count >= MAX_TASKS) {
    return NULL; // Too many tasks
  }

  // Allocate task structure
  struct task *new_task = (struct task *)kmalloc(sizeof(struct task));
  if (new_task == NULL) {
    return NULL;
  }

  // Initialize task
  new_task->id = next_task_id++;
  new_task->state = TASK_READY;
  extern void task_start_asm(void);

  new_task->priority = priority;
  new_task->time_slice = DEFAULT_TIME_SLICE;
  new_task->page_directory = 0; // Use kernel page directory for now
  new_task->next = NULL;
  new_task->next_in_queue = NULL;
  new_task->wake_ticks = 0;

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

  // Set up initial stack frame for context_switch_asm
  // context_switch_asm expects on stack:
  // [esp+20] entry point (for task_start_asm to pop into eax)
  // [esp+16] eip (return address -> task_start_asm)
  // [esp+12] ebp
  // [esp+8]  ebx
  // [esp+4]  esi
  // [esp+0]  edi

  // We need 6 dwords (24 bytes)
  uint32_t *stack = (uint32_t *)(stack_top - 24);

  stack[5] = (uint32_t)entry;          // For 'pop eax' in task_start_asm
  stack[4] = (uint32_t)task_start_asm; // eip (where ret will jump)
  stack[3] = 0;                        // ebp
  stack[2] = 0;                        // ebx
  stack[1] = 0;                        // esi
  stack[0] = 0;                        // edi

  new_task->esp = (uint32_t)stack;
  new_task->ebp = 0;
  new_task->eip = (uint32_t)task_start_asm;

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
extern void task_switch_asm(uint32_t *old_esp, uint32_t new_esp);

void task_switch(void) {
  if (task_list == NULL) {
    return; // No tasks to switch to
  }

  // Find next ready task
  struct task *next = current_task;
  if (next == NULL) {
    next = task_list;
  } else {
    next = next->next;
  }

  // Process sleeping tasks
  struct task *checking = task_list;
  uint32_t current_ticks = timer_get_ticks();
  do {
    if (checking->state == TASK_BLOCKED && checking->wake_ticks != 0) {
      if (current_ticks >= checking->wake_ticks) {
        checking->state = TASK_READY;
        checking->wake_ticks = 0;
      }
    }
    checking = checking->next;
  } while (checking != task_list);

  // Round-robin: find next ready task
  uint32_t attempts = 0;
  while (next->state != TASK_READY && attempts < task_count) {
    next = next->next;
    attempts++;
  }

  if (next->state != TASK_READY) {
    return; // No ready tasks
  }

  // Save current task pointer before switching so we can update its esp
  struct task *prev = current_task;

  // Switch to next task
  current_task = next;
  current_task->state = TASK_RUNNING;
  current_task->time_slice = DEFAULT_TIME_SLICE;

  // Load new task's stack pointer and switch
  uint32_t new_esp = current_task->esp;

  if (prev != current_task) {
    if (prev->state == TASK_RUNNING) {
      prev->state = TASK_READY;
    }
    // task_switch_asm will save the current esp into prev->esp
    // and load new_esp into esp
    task_switch_asm(&prev->esp, new_esp);
  }
}

struct task *task_get_current(void) { return current_task; }

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
  if (current_task == NULL) {
    return;
  }
  uint32_t target = timer_get_ticks() + (milliseconds * TIMER_FREQUENCY / 1000);
  task_sleep_until(target);
}

void task_sleep_until(uint32_t target_ticks) {
  if (current_task == NULL) {
    return;
  }

  current_task->state = TASK_BLOCKED;
  current_task->wake_ticks = target_ticks;
  task_switch();
}

void task_block(struct task **wait_queue) {
  if (current_task == NULL || wait_queue == NULL) {
    return;
  }

  current_task->state = TASK_BLOCKED;
  current_task->wake_ticks = 0;
  current_task->next_in_queue = NULL;

  // Add to the end of the wait queue
  if (*wait_queue == NULL) {
    *wait_queue = current_task;
  } else {
    struct task *curr = *wait_queue;
    while (curr->next_in_queue != NULL) {
      curr = curr->next_in_queue;
    }
    curr->next_in_queue = current_task;
  }

  task_switch();
}

void task_unblock(struct task **wait_queue) {
  if (wait_queue == NULL || *wait_queue == NULL) {
    return;
  }

  struct task *task_to_wake = *wait_queue;
  *wait_queue = task_to_wake->next_in_queue;
  task_to_wake->next_in_queue = NULL;

  task_wake(task_to_wake);
}

void task_wake(struct task *task) {
  if (task != NULL && task->state == TASK_BLOCKED) {
    task->state = TASK_READY;
    task->wake_ticks = 0;
  }
}

struct task *task_get_by_id(uint32_t id) {
  if (task_list == NULL) {
    return NULL;
  }

  struct task *current = task_list;
  do {
    if (current->id == id) {
      return current;
    }
    current = current->next;
  } while (current != task_list);

  return NULL;
}
