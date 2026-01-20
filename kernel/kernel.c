// FlowDay-OS Kernel
// Основной файл ядра

#include "kernel.h"
#include "multiboot.h"
#include "vga.h"
#include "string.h"

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
        vga_puts("Memory: ");
        vga_putdec(mb_info->mem_lower);
        vga_puts(" KB (lower), ");
        vga_putdec(mb_info->mem_upper);
        vga_puts(" KB (upper)\n");
    }
    
    vga_puts("\n");
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("Welcome to FlowDay-OS!\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("System is ready.\n");
    vga_puts("\n");
    
    // Основной цикл ядра
    while (1) {
        asm volatile("hlt"); // Ожидание прерывания
    }
}
