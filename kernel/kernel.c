// FlowDay-OS Kernel
// Основной файл ядра

#include "kernel.h"
#include "multiboot.h"
#include "vga.h"
#include "string.h"
#include "pmm.h"
#include "paging.h"
#include "heap.h"
#include "idt.h"
#include "pic.h"
#include "timer.h"
#include "keyboard.h"
#include "task.h"
#include "syscall.h"
#include "ata.h"
#include "fs.h"

// Глобальные переменные
struct multiboot_info* mb_info = 0;

void kernel_main(unsigned long magic, unsigned long addr) {
    // Проверка Multiboot magic number
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        // Если не Multiboot, выводим ошибку
        vga_clear();
        vga_puts("ERROR: Invalid Multiboot magic number: 0x");
        vga_puthex(magic);
        vga_puts("\n");
        return;
    }
    
    // Сохраняем указатель на multiboot info
    mb_info = (struct multiboot_info*)addr;
    
    // Инициализация VGA
    vga_init();
    vga_clear();
    
    // Инициализация управления памятью
    vga_puts("Initializing memory management...\n");
    pmm_init(mb_info);
    paging_init();
    heap_init();
    
    // Инициализация прерываний
    vga_puts("Initializing interrupts...\n");
    idt_init();
    pic_init();
    
    // Инициализация драйверов устройств
    vga_puts("Initializing device drivers...\n");
    timer_init(TIMER_FREQUENCY);
    keyboard_init();
    
    // Инициализация многозадачности
    vga_puts("Initializing multitasking...\n");
    task_init();
    syscall_init();
    
    // Инициализация файловой системы
    vga_puts("Initializing file system...\n");
    ata_init();
    fs_init();
    
    // Включаем прерывания
    asm volatile("sti");
    vga_puts("Interrupts enabled.\n");
    vga_puts("Timer initialized at ");
    vga_putdec(TIMER_FREQUENCY);
    vga_puts(" Hz\n");
    vga_puts("Keyboard driver ready.\n");
    vga_puts("Multitasking ready.\n");
    
    // Приветственное сообщение
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("========================================\n");
    vga_puts("    FlowDay-OS Kernel v0.1\n");
    vga_puts("========================================\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("\n");
    
    vga_puts("Kernel initialized successfully!\n");
    vga_puts("Multiboot magic: 0x");
    vga_puthex(magic);
    vga_puts("\n");
    
    // Вывод информации о памяти
    if (mb_info->flags & 0x01) {
        vga_puts("Physical Memory: ");
        vga_putdec(mb_info->mem_lower);
        vga_puts(" KB (lower), ");
        vga_putdec(mb_info->mem_upper);
        vga_puts(" KB (upper)\n");
    }
    
    // Вывод информации о PMM
    vga_puts("PMM: Total pages: ");
    vga_putdec(pmm_get_total_pages());
    vga_puts(", Free pages: ");
    vga_putdec(pmm_get_free_pages());
    vga_puts("\n");
    
    // Тест heap allocator
    vga_puts("Testing heap allocator...\n");
    void* ptr1 = kmalloc(256);
    void* ptr2 = kmalloc(512);
    void* ptr3 = kmalloc(1024);
    
    if (ptr1 && ptr2 && ptr3) {
        vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
        vga_puts("Heap allocator: OK\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        
        kfree(ptr2);
        kfree(ptr1);
        kfree(ptr3);
        vga_puts("Heap deallocation: OK\n");
    } else {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("Heap allocator: FAILED\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    }
    
    vga_puts("\n");
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("Welcome to FlowDay-OS!\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("System is ready.\n");
    vga_puts("Type something on your keyboard...\n");
    vga_puts("\n");
    
    // Основной цикл ядра
    uint32_t last_second = 0;
    while (1) {
        asm volatile("hlt"); // Ожидание прерывания
        
        // Обработка клавиатуры
        if (keyboard_is_key_available()) {
            char key = keyboard_get_key();
            if (key == '\b') {
                // Backspace - удаляем последний символ
                vga_putchar('\b');
                vga_putchar(' ');
                vga_putchar('\b');
            } else if (key == '\n') {
                // Enter - новая строка
                vga_putchar('\n');
            } else if (key >= 32 && key < 127) {
                // Печатаемый символ
                vga_putchar(key);
            }
        }
        
        // Показываем время каждую секунду
        uint32_t current_second = timer_get_ms() / 1000;
        if (current_second != last_second) {
            last_second = current_second;
            // Можно добавить вывод времени, если нужно
        }
    }
}
