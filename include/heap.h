// Heap Allocator
// Динамическое выделение памяти (malloc/free)

#ifndef HEAP_H
#define HEAP_H

#include "types.h"

// Инициализация heap
void heap_init(void);

// Выделение памяти
void* kmalloc(size_t size);

// Выделение памяти с выравниванием
void* kmalloc_aligned(size_t size, size_t alignment);

// Освобождение памяти
void kfree(void* ptr);

// Изменить размер блока памяти
void* krealloc(void* ptr, size_t size);

// Получить размер выделенного блока
size_t ksize(void* ptr);

#endif // HEAP_H
