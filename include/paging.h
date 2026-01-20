// Paging (Виртуальная память)
// Управление страничной адресацией

#ifndef PAGING_H
#define PAGING_H

#include "types.h"

// Флаги страницы
#define PAGE_PRESENT    0x001
#define PAGE_WRITABLE   0x002
#define PAGE_USER       0x004
#define PAGE_WRITETHROUGH 0x008
#define PAGE_CACHE_DISABLE 0x010
#define PAGE_ACCESSED   0x020
#define PAGE_DIRTY      0x040
#define PAGE_SIZE_4MB   0x080
#define PAGE_GLOBAL     0x100

// Инициализация paging
void paging_init(void);

// Создать mapping виртуальной страницы на физическую
void paging_map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);

// Удалить mapping страницы
void paging_unmap_page(uint32_t virtual_addr);

// Получить физический адрес виртуальной страницы
uint32_t paging_get_physical(uint32_t virtual_addr);

// Включить paging
void paging_enable(void);

// Отключить paging
void paging_disable(void);

#endif // PAGING_H
