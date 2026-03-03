// Synchronization Primitives
// Mutexes and Semaphores

#ifndef SYNC_H
#define SYNC_H

#include "task.h"
#include "types.h"

// Mutex structure
typedef struct {
  uint32_t locked;
  struct task *owner;
  struct task *wait_queue;
} mutex_t;

// Semaphore structure
typedef struct {
  uint32_t count;
  struct task *wait_queue;
} semaphore_t;

// Mutex API
void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);
int mutex_trylock(mutex_t *mutex);

// Semaphore API
void sem_init(semaphore_t *sem, uint32_t value);
void sem_wait(semaphore_t *sem);
void sem_post(semaphore_t *sem);

#endif // SYNC_H
