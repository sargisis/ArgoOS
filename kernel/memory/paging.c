// Paging Implementation
// Виртуальная память с 4KB страницами

#include "paging.h"
#include "pmm.h"
#include "string.h"

// Page Directory и Page Tables должны быть выровнены по 4KB
static uint32_t page_directory[1024] __attribute__((aligned(4096)));
static uint32_t first_page_table[1024] __attribute__((aligned(4096)));

// Создать новую page table
static uint32_t* paging_create_page_table(void) {
    uint32_t* table = (uint32_t*)pmm_alloc_page();
    if (table == NULL) {
        return NULL;
    }
    
    // Очищаем таблицу
    memset(table, 0, 1024 * sizeof(uint32_t));
    return table;
}

void paging_init(void) {
    // Очищаем page directory
    memset(page_directory, 0, 1024 * sizeof(uint32_t));
    
    // Инициализируем первую page table для первых 4MB
    // Identity mapping (виртуальный адрес = физический)
    for (uint32_t i = 0; i < 1024; i++) {
        first_page_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_WRITABLE;
    }
    
    // Устанавливаем первую запись в page directory
    page_directory[0] = ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_WRITABLE;
    
    // Остальные записи в page directory помечаем как несуществующие
    for (uint32_t i = 1; i < 1024; i++) {
        page_directory[i] = 0;
    }
    
    // Загружаем page directory в CR3
    asm volatile("mov %0, %%cr3" :: "r"(page_directory));
    
    // Включаем paging
    paging_enable();
}

void paging_map_page(uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
    uint32_t pd_index = virtual_addr >> 22;  // Индекс в page directory
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;  // Индекс в page table
    
    // Проверяем, существует ли page table
    if (!(page_directory[pd_index] & PAGE_PRESENT)) {
        // Создаем новую page table
        uint32_t* page_table = paging_create_page_table();
        if (page_table == NULL) {
            return; // Не удалось выделить память
        }
        
        page_directory[pd_index] = ((uint32_t)page_table) | PAGE_PRESENT | PAGE_WRITABLE;
    }
    
    // Получаем указатель на page table
    uint32_t* page_table = (uint32_t*)(page_directory[pd_index] & ~0xFFF);
    
    // Устанавливаем запись в page table
    page_table[pt_index] = physical_addr | flags;
    
    // Обновляем TLB (Translation Lookaside Buffer)
    asm volatile("invlpg (%0)" :: "r"(virtual_addr) : "memory");
}

void paging_unmap_page(uint32_t virtual_addr) {
    uint32_t pd_index = virtual_addr >> 22;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;
    
    if (page_directory[pd_index] & PAGE_PRESENT) {
        uint32_t* page_table = (uint32_t*)(page_directory[pd_index] & ~0xFFF);
        page_table[pt_index] = 0;
        
        // Обновляем TLB
        asm volatile("invlpg (%0)" :: "r"(virtual_addr) : "memory");
    }
}

uint32_t paging_get_physical(uint32_t virtual_addr) {
    uint32_t pd_index = virtual_addr >> 22;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;
    
    if (!(page_directory[pd_index] & PAGE_PRESENT)) {
        return 0; // Страница не существует
    }
    
    uint32_t* page_table = (uint32_t*)(page_directory[pd_index] & ~0xFFF);
    
    if (!(page_table[pt_index] & PAGE_PRESENT)) {
        return 0; // Страница не существует
    }
    
    return (page_table[pt_index] & ~0xFFF) + (virtual_addr & 0xFFF);
}

void paging_enable(void) {
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000; // Устанавливаем бит PG (Paging Enable)
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

void paging_disable(void) {
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~0x80000000; // Сбрасываем бит PG
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}
