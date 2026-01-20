// Heap Allocator Implementation
// Простой heap allocator с linked list

#include "heap.h"
#include "pmm.h"
#include "paging.h"
#include "string.h"

#define HEAP_START 0x400000  // Начинаем heap с 4MB
#define HEAP_INITIAL_SIZE 0x100000  // Начальный размер 1MB
#define HEAP_MAX_SIZE 0x4000000  // Максимальный размер 64MB

// Структура блока памяти
struct heap_block {
    size_t size;
    int free;
    struct heap_block* next;
    struct heap_block* prev;
};

static struct heap_block* heap_start = NULL;
static uint32_t heap_size = 0;
static uint32_t heap_used = 0;

// Найти свободный блок подходящего размера
static struct heap_block* find_free_block(size_t size) {
    struct heap_block* current = heap_start;
    
    while (current != NULL) {
        if (current->free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

// Разделить блок на два, если возможно
static void split_block(struct heap_block* block, size_t size) {
    if (block->size - size >= sizeof(struct heap_block) + 4) {
        struct heap_block* new_block = (struct heap_block*)((char*)block + sizeof(struct heap_block) + size);
        new_block->size = block->size - size - sizeof(struct heap_block);
        new_block->free = 1;
        new_block->next = block->next;
        new_block->prev = block;
        
        if (block->next) {
            block->next->prev = new_block;
        }
        
        block->next = new_block;
        block->size = size;
    }
}

// Объединить свободные блоки
static void merge_blocks(struct heap_block* block) {
    // Объединяем с следующим блоком, если он свободен
    if (block->next && block->next->free) {
        block->size += sizeof(struct heap_block) + block->next->size;
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }
    
    // Объединяем с предыдущим блоком, если он свободен
    if (block->prev && block->prev->free) {
        block->prev->size += sizeof(struct heap_block) + block->size;
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
        block = block->prev;
    }
}

// Расширить heap
static int expand_heap(size_t size) {
    if (heap_size + size > HEAP_MAX_SIZE) {
        return 0; // Достигнут максимум
    }
    
    // Выделяем физические страницы
    uint32_t pages_needed = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    
    for (uint32_t i = 0; i < pages_needed; i++) {
        void* page = pmm_alloc_page();
        if (page == NULL) {
            return 0; // Не удалось выделить страницу
        }
        
        // Создаем mapping для новой страницы
        uint32_t virtual_addr = HEAP_START + heap_size + (i * PAGE_SIZE);
        paging_map_page(virtual_addr, (uint32_t)page, PAGE_PRESENT | PAGE_WRITABLE);
    }
    
    heap_size += pages_needed * PAGE_SIZE;
    return 1;
}

void heap_init(void) {
    // Выделяем начальные страницы для heap
    uint32_t pages_needed = (HEAP_INITIAL_SIZE + PAGE_SIZE - 1) / PAGE_SIZE;
    
    for (uint32_t i = 0; i < pages_needed; i++) {
        void* page = pmm_alloc_page();
        if (page == NULL) {
            return; // Ошибка выделения памяти
        }
        
        // Создаем identity mapping для начального heap
        uint32_t virtual_addr = HEAP_START + (i * PAGE_SIZE);
        paging_map_page(virtual_addr, (uint32_t)page, PAGE_PRESENT | PAGE_WRITABLE);
    }
    
    heap_size = HEAP_INITIAL_SIZE;
    
    // Инициализируем первый блок
    heap_start = (struct heap_block*)HEAP_START;
    heap_start->size = HEAP_INITIAL_SIZE - sizeof(struct heap_block);
    heap_start->free = 1;
    heap_start->next = NULL;
    heap_start->prev = NULL;
    
    heap_used = 0;
}

void* kmalloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    // Выравниваем размер на 4 байта
    size = (size + 3) & ~3;
    
    struct heap_block* block = find_free_block(size);
    
    if (block == NULL) {
        // Нужно расширить heap
        size_t expand_size = size + sizeof(struct heap_block);
        if (!expand_heap(expand_size)) {
            return NULL; // Не удалось расширить heap
        }
        
        // Ищем последний блок
        block = heap_start;
        while (block->next != NULL) {
            block = block->next;
        }
        
        // Создаем новый блок в конце
        if (block->free) {
            block->size += expand_size;
        } else {
            struct heap_block* new_block = (struct heap_block*)((char*)block + sizeof(struct heap_block) + block->size);
            new_block->size = expand_size - sizeof(struct heap_block);
            new_block->free = 1;
            new_block->next = NULL;
            new_block->prev = block;
            block->next = new_block;
            block = new_block;
        }
    }
    
    // Разделяем блок, если он слишком большой
    split_block(block, size);
    
    block->free = 0;
    heap_used += size;
    
    return (void*)((char*)block + sizeof(struct heap_block));
}

void kfree(void* ptr) {
    if (ptr == NULL) {
        return;
    }
    
    struct heap_block* block = (struct heap_block*)((char*)ptr - sizeof(struct heap_block));
    
    if (block->free) {
        return; // Уже освобожден
    }
    
    block->free = 1;
    heap_used -= block->size;
    
    // Объединяем с соседними свободными блоками
    merge_blocks(block);
}

void* krealloc(void* ptr, size_t size) {
    if (ptr == NULL) {
        return kmalloc(size);
    }
    
    if (size == 0) {
        kfree(ptr);
        return NULL;
    }
    
    struct heap_block* block = (struct heap_block*)((char*)ptr - sizeof(struct heap_block));
    
    if (block->size >= size) {
        return ptr; // Блок уже достаточно большой
    }
    
    // Выделяем новый блок
    void* new_ptr = kmalloc(size);
    if (new_ptr == NULL) {
        return NULL;
    }
    
    // Копируем данные
    memcpy(new_ptr, ptr, block->size);
    
    // Освобождаем старый блок
    kfree(ptr);
    
    return new_ptr;
}

size_t ksize(void* ptr) {
    if (ptr == NULL) {
        return 0;
    }
    
    struct heap_block* block = (struct heap_block*)((char*)ptr - sizeof(struct heap_block));
    return block->size;
}
