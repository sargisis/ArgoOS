// Physical Memory Manager Implementation

#include "pmm.h"
#include "multiboot.h"
#include "string.h"
#include "vga.h"

#define BITMAP_SIZE 32768  // Максимум 128MB памяти (32768 страниц * 4KB)
#define MAX_MEMORY 0x10000000  // 256MB максимум

static uint32_t* bitmap = 0;
static uint32_t total_pages = 0;
static uint32_t used_pages = 0;
static uint32_t bitmap_size = 0;

// Установить бит в bitmap
static void bitmap_set(uint32_t bit) {
    bitmap[bit / 32] |= (1 << (bit % 32));
}

// Сбросить бит в bitmap
static void bitmap_clear(uint32_t bit) {
    bitmap[bit / 32] &= ~(1 << (bit % 32));
}

// Проверить бит в bitmap
static int bitmap_test(uint32_t bit) {
    return (bitmap[bit / 32] & (1 << (bit % 32))) != 0;
}

// Найти первую свободную страницу
static uint32_t bitmap_first_free(void) {
    for (uint32_t i = 0; i < bitmap_size; i++) {
        if (bitmap[i] != 0xFFFFFFFF) {
            // Найден не полностью занятый блок
            for (uint32_t j = 0; j < 32; j++) {
                if (!(bitmap[i] & (1 << j))) {
                    return i * 32 + j;
                }
            }
        }
    }
    return 0xFFFFFFFF; // Нет свободных страниц
}

void pmm_init(struct multiboot_info* mb_info) {
    // Вычисляем общее количество памяти
    uint32_t total_memory = 0;
    
    if (mb_info->flags & 0x01) {
        // Используем информацию из multiboot
        total_memory = (mb_info->mem_lower + mb_info->mem_upper) * 1024;
    } else {
        // По умолчанию 64MB
        total_memory = 64 * 1024 * 1024;
    }
    
    // Ограничиваем максимумом
    if (total_memory > MAX_MEMORY) {
        total_memory = MAX_MEMORY;
    }
    
    total_pages = total_memory / PAGE_SIZE;
    bitmap_size = (total_pages + 31) / 32; // Размер bitmap в словах
    
    // Размещаем bitmap после ядра (примерно по адресу 0x200000)
    bitmap = (uint32_t*)0x200000;
    
    // Инициализируем bitmap нулями (все страницы свободны)
    memset(bitmap, 0, bitmap_size * sizeof(uint32_t));
    
    // Помечаем первые 4MB как занятые (ядро и bitmap)
    uint32_t kernel_end = 0x200000 + (bitmap_size * sizeof(uint32_t));
    uint32_t reserved_pages = PAGE_NUMBER(PAGE_ALIGN(kernel_end));
    
    for (uint32_t i = 0; i < reserved_pages && i < total_pages; i++) {
        bitmap_set(i);
        used_pages++;
    }
    
    // Обрабатываем memory map из multiboot, если доступен
    if (mb_info->flags & 0x40) { // Флаг наличия memory map
        struct multiboot_mmap_entry* mmap = (struct multiboot_mmap_entry*)mb_info->mmap_addr;
        uint32_t mmap_end = mb_info->mmap_addr + mb_info->mmap_length;
        
        while ((uint32_t)mmap < mmap_end) {
            // Тип 1 = доступная память, остальные = занятая
            if (mmap->type != 1) {
                uint32_t addr = mmap->addr_low;
                uint32_t len = mmap->len_low;
                uint32_t start_page = PAGE_NUMBER(addr);
                uint32_t end_page = PAGE_NUMBER(addr + len);
                
                // Помечаем страницы как занятые
                for (uint32_t i = start_page; i < end_page && i < total_pages; i++) {
                    if (!bitmap_test(i)) {
                        bitmap_set(i);
                        used_pages++;
                    }
                }
            }
            mmap = (struct multiboot_mmap_entry*)((uint32_t)mmap + mmap->size + sizeof(mmap->size));
        }
    }
}

void* pmm_alloc_page(void) {
    uint32_t page = bitmap_first_free();
    
    if (page == 0xFFFFFFFF || page >= total_pages) {
        return NULL; // Нет свободных страниц
    }
    
    bitmap_set(page);
    used_pages++;
    
    return (void*)(page * PAGE_SIZE);
}

void pmm_free_page(void* page) {
    if (page == NULL) {
        return;
    }
    
    uint32_t page_num = PAGE_NUMBER((uint32_t)page);
    
    if (page_num >= total_pages) {
        return; // Неверный адрес
    }
    
    if (bitmap_test(page_num)) {
        bitmap_clear(page_num);
        used_pages--;
    }
}

uint32_t pmm_get_free_pages(void) {
    return total_pages - used_pages;
}

uint32_t pmm_get_total_pages(void) {
    return total_pages;
}
