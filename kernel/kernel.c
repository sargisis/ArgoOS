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
#include "serial.h"
#include "task.h"
#include "syscall.h"
#include "ata.h"
#include "fs.h"
#include "shell.h"

// Forward declaration
extern char keyboard_scancode_to_ascii(uint8_t scancode);

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
    serial_init();
    serial_puts("Serial port initialized (COM1)\n");
    serial_puts("Initializing device drivers...\n");
    timer_init(TIMER_FREQUENCY);
    serial_puts("Timer initialized\n");
    keyboard_init();
    serial_puts("Keyboard driver ready\n");
    
    // Инициализация многозадачности
    vga_puts("Initializing multitasking...\n");
    serial_puts("Initializing multitasking...\n");
    task_init();
    syscall_init();
    serial_puts("Multitasking ready\n");
    
    // Инициализация файловой системы
    vga_puts("Initializing file system...\n");
    serial_puts("Initializing file system...\n");
    ata_init();
    serial_puts("ATA driver initialized\n");
    fs_init();
    serial_puts("File system initialized\n");
    
    // Включаем прерывания
    asm volatile("sti");
    vga_puts("Interrupts enabled.\n");
    serial_puts("Interrupts enabled\n");
    
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
    vga_puts("\n");
    
    // Очистка буфера клавиатуры при старте
    // Читаем все старые данные из буфера
    for (int i = 0; i < 10; i++) {
        uint8_t status;
        asm volatile("inb %1, %0" : "=a"(status) : "Nd"(0x64));
        if (status & 0x01) {
            uint8_t dummy;
            asm volatile("inb %1, %0" : "=a"(dummy) : "Nd"(0x60));
            (void)dummy;
        } else {
            break;
        }
    }
    
    // Используем serial port для ввода/вывода
    serial_puts("\n=== FlowDay-OS Shell (Serial) ===\n");
    serial_puts("Using serial port for input/output.\n");
    serial_puts("Type commands here:\n\n");
    
    // Инициализация shell
    shell_init();
    serial_puts("Shell initialized\n");
    
    // Основной цикл через serial port
    char input_line[256] = {0};
    int input_pos = 0;
    
    serial_puts("FlowDay-OS> ");
    
    while (1) {
        asm volatile("sti");
        
        // Читаем из serial port
        if (serial_is_data_available()) {
            char c = serial_getchar();
            
            if (c == '\n' || c == '\r') {
                // Enter - выполняем команду
                serial_puts("\n");
                if (input_pos > 0) {
                    input_line[input_pos] = '\0';
                    // Выполняем команду через shell
                    shell_process_command(input_line);
                    input_pos = 0;
                    memset(input_line, 0, 256);
                }
                serial_puts("FlowDay-OS> ");
            } else if (c == '\b' || c == 127) {
                // Backspace
                if (input_pos > 0) {
                    input_pos--;
                    input_line[input_pos] = '\0';
                    serial_puts("\b \b");
                }
            } else if (c >= 32 && c < 127) {
                // Печатаемый символ
                if (input_pos < 255) {
                    input_line[input_pos++] = c;
                    serial_putchar(c);
                }
            }
        }
        
        // Небольшая задержка
        asm volatile("pause");
    }
}
