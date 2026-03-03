// Task/Process Management
// Multitasking support

#ifndef TASK_H
#define TASK_H

#include "paging.h"
#include "types.h"

// Task states
#define TASK_RUNNING 0
#define TASK_READY 1
#define TASK_BLOCKED 2
#define TASK_ZOMBIE 3

// Maximum number of tasks
#define MAX_TASKS 256

// Task structure
struct task {
  uint32_t id;                // Task ID
  uint32_t esp;               // Stack pointer
  uint32_t ebp;               // Base pointer
  uint32_t eip;               // Instruction pointer
  uint32_t page_directory;    // Page directory address
  uint32_t state;             // Task state
  uint32_t priority;          // Task priority
  uint32_t time_slice;        // Remaining time slice
  struct task *next;          // Next task in list
  struct task *next_in_queue; // Next task in wait queue
  uint32_t wake_ticks;        // Ticks when to wake up (0 if not sleeping)
  char name[32];              // Task name
};

// Task context (registers saved during context switch)
struct task_context {
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t esp;
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  uint32_t eip;
  uint32_t cs;
  uint32_t eflags;
};

// Initialize task manager
void task_init(void);

// Create a new task
struct task *task_create(void (*entry)(void), const char *name,
                         uint32_t priority);

// Switch to next task
void task_switch(void);

// Get current task
struct task *task_get_current(void);

// Yield CPU to next task
void task_yield(void);

// Exit current task
void task_exit(void);

// Sleep current task for specified milliseconds
void task_sleep(uint32_t milliseconds);

// Sleep until specific tick count
void task_sleep_until(uint32_t target_ticks);

// Block the current task on a wait queue
void task_block(struct task **wait_queue);

// Unblock a single task from a wait queue
void task_unblock(struct task **wait_queue);

// Wake up a task (move to ready state)
void task_wake(struct task *task);

// Get task by ID
struct task *task_get_by_id(uint32_t id);

#endif // TASK_H
