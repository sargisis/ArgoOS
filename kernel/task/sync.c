// Synchronization Primitives Implementation

#include "sync.h"

// Disable interrupts internally to ensure atomicity
static inline uint32_t disable_interrupts(void) {
  uint32_t eflags;
  asm volatile("pushf; pop %0; cli" : "=r"(eflags));
  return eflags;
}

static inline void restore_interrupts(uint32_t eflags) {
  asm volatile("push %0; popf" ::"r"(eflags));
}

// Mutex Implementation

void mutex_init(mutex_t *mutex) {
  if (mutex) {
    mutex->locked = 0;
    mutex->owner = NULL;
    mutex->wait_queue = NULL;
  }
}

void mutex_lock(mutex_t *mutex) {
  if (!mutex)
    return;

  struct task *current = task_get_current();
  if (!current)
    return;

  uint32_t flags = disable_interrupts();

  if (mutex->locked == 0) {
    // Mutex is free, take it
    mutex->locked = 1;
    mutex->owner = current;
    restore_interrupts(flags);
  } else {
    // Mutex is taken, check if we already own it (not recursive, should
    // probably deadlock or panic)
    if (mutex->owner == current) {
      restore_interrupts(flags);
      return; // Avoiding deadlock for now by returning, though typical
              // non-recursive mutex would panic/deadlock
    }

    // Block the current task on the wait queue
    task_block(&mutex->wait_queue);
    // Note: task_block switches task, so we are blocked here
    // When we wake up, it means unlock gave us the mutex.
    // The unblock logic in unlock transfers ownership.
    restore_interrupts(flags);
  }
}

void mutex_unlock(mutex_t *mutex) {
  if (!mutex)
    return;

  struct task *current = task_get_current();

  uint32_t flags = disable_interrupts();

  if (mutex->locked && mutex->owner == current) {
    if (mutex->wait_queue != NULL) {
      // There are tasks waiting. Wake one up and give it the lock.
      struct task *next_owner = mutex->wait_queue;

      // Unblock it (removes from wait queue and makes it ready)
      task_unblock(&mutex->wait_queue);

      // Transfer ownership
      mutex->owner = next_owner;
      // locked remains 1
    } else {
      // No one waiting, just unlock
      mutex->locked = 0;
      mutex->owner = NULL;
    }
  }

  restore_interrupts(flags);
}

int mutex_trylock(mutex_t *mutex) {
  if (!mutex)
    return 0;

  struct task *current = task_get_current();
  if (!current)
    return 0;

  uint32_t flags = disable_interrupts();

  int success = 0;
  if (mutex->locked == 0) {
    mutex->locked = 1;
    mutex->owner = current;
    success = 1;
  }

  restore_interrupts(flags);
  return success;
}

// Semaphore Implementation

void sem_init(semaphore_t *sem, uint32_t value) {
  if (sem) {
    sem->count = value;
    sem->wait_queue = NULL;
  }
}

void sem_wait(semaphore_t *sem) {
  if (!sem)
    return;

  uint32_t flags = disable_interrupts();

  if (sem->count > 0) {
    sem->count--;
    restore_interrupts(flags);
  } else {
    // Block until sem_post is called
    task_block(&sem->wait_queue);
    // Once we wake up, we have acquired the semaphore.
    restore_interrupts(flags);
  }
}

void sem_post(semaphore_t *sem) {
  if (!sem)
    return;

  uint32_t flags = disable_interrupts();

  if (sem->wait_queue != NULL) {
    // Wake up one waiting task
    // We don't increment the count, we just pass the signal to the waiting task
    task_unblock(&sem->wait_queue);
  } else {
    sem->count++;
  }

  restore_interrupts(flags);
}
