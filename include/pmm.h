// Physical Memory Manager (PMM)
// Управление физической памятью

#ifndef PMM_H
#define PMM_H

#include "types.h"
#include "multiboot.h"

// Размер страницы (4 KB)
#define PAGE_SIZE 4096

// Макросы для работы со страницами
#define PAGE_ALIGN(addr) (((addr) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_ALIGN_DOWN(addr) ((addr) & ~(PAGE_SIZE - 1))
#define PAGE_NUMBER(addr) ((addr) / PAGE_SIZE)

// Инициализация PMM
void pmm_init(struct multiboot_info* mb_info);

// Выделение физической страницы
void* pmm_alloc_page(void);

// Освобождение физической страницы
void pmm_free_page(void* page);

// Получить количество свободных страниц
uint32_t pmm_get_free_pages(void);

// Получить общее количество страниц
uint32_t pmm_get_total_pages(void);

#endif // PMM_H
