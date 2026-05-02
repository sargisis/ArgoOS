// ArgOS Kernel
// Основной файл ядра

#include "kernel.h"
#include "multiboot.h"
#include "gdt.h"
#include "panic.h"
#include "vga.h"
#include "graphics.h"
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
        PANIC("Invalid Multiboot magic number");
    }
    
    // Сохраняем указатель на multiboot info
    mb_info = (struct multiboot_info*)addr;
    
    // Инициализация базовых структур CPU
    gdt_init();
    
    // Инициализация VGA (остается для совместимости)
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
    timer_init(TIMER_FREQUENCY);
    serial_puts("Timer initialized\n");
    keyboard_init();
    serial_puts("Keyboard driver ready\n");
    
    // Инициализация графики (ПОСЛЕ paging!)
    graphics_init(mb_info);
    serial_puts("Graphics initialized\n");
    
    // Инициализация многозадачности
    serial_puts("Initializing multitasking...\n");
    task_init();
    syscall_init();
    serial_puts("Multitasking ready\n");
    
    // Инициализация файловой системы
    serial_puts("Initializing file system...\n");
    ata_init();
    serial_puts("ATA driver initialized\n");
    fs_init();
    serial_puts("File system initialized\n");
    
    // Включаем прерывания
    asm volatile("sti");
    serial_puts("Interrupts enabled\n");
    
    // Рисуем графический интерфейс (GUI)
    int sw = graphics_get_width();
    int sh = graphics_get_height();
    
    if (sw > 0 && sh > 0) {
        // 1. Обои рабочего стола (Темный космос)
        graphics_clear(COLOR_DARK_BG);
        
        // 2. Панель задач снизу (Taskbar)
        graphics_draw_rect(0, sh - 40, sw, 40, 0x111111);
        
        // 3. Кнопка "Пуск" (ArgOS Logo)
        graphics_draw_rect(10, sh - 35, 60, 30, COLOR_CYAN);
        graphics_draw_string(18, sh - 30, "ArgOS", 0x000000);
        
        // Часы на панели задач
        graphics_draw_string(sw - 60, sh - 28, "04:30", COLOR_WHITE);
        
        // 4. Главное окно по центру
        int win_w = 400;
        int win_h = 300;
        int win_x = (sw - win_w) / 2;
        int win_y = (sh - win_h) / 2;
        
        // Тень окна
        graphics_draw_rect(win_x + 5, win_y + 5, win_w, win_h, 0x0A0A15);
        // Фон окна
        graphics_draw_rect(win_x, win_y, win_w, win_h, 0x222233);
        // Заголовок окна
        graphics_draw_rect(win_x, win_y, win_w, 25, 0x333355);
        graphics_draw_string(win_x + 8, win_y + 5, "Welcome to ArgOS", COLOR_WHITE);
        // Кнопка закрытия окна
        graphics_draw_rect(win_x + win_w - 25, win_y + 5, 15, 15, COLOR_RED);
        graphics_draw_string(win_x + win_w - 22, win_y + 5, "X", COLOR_WHITE);
        
        // Текст внутри окна
        graphics_draw_string(win_x + 20, win_y + 50, "ArgOS Kernel v0.1", COLOR_CYAN);
        graphics_draw_string(win_x + 20, win_y + 80, "Graphics: 800x600 32-bit", COLOR_GREEN);
        graphics_draw_string(win_x + 20, win_y + 110, "Status: Running", COLOR_GREEN);
        graphics_draw_string(win_x + 20, win_y + 150, "Your own Operating System!", 0xAAAAFF);
        graphics_draw_string(win_x + 20, win_y + 180, "Built from scratch in C.", 0x888899);
        
        serial_puts("GUI rendered!\n");
    }
    
    // Телеметрия
    serial_puts("\n[TELEMETRY] {\"system\": \"ArgOS\", \"status\": \"online\", \"version\": \"0.1\"}\n");
    
    // Shell через serial port
    serial_puts("\n=== ArgOS Shell (Serial) ===\n");
    serial_puts("Type commands here:\n\n");
    
    shell_init();
    serial_puts("Shell initialized\n");
    
    // Основной цикл через serial port
    char input_line[256] = {0};
    int input_pos = 0;
    
    serial_puts("ArgOS> ");
    
    while (1) {
        asm volatile("sti");
        
        if (serial_is_data_available()) {
            char c = serial_getchar();
            
            if (c == '\n' || c == '\r') {
                serial_puts("\n");
                if (input_pos > 0) {
                    input_line[input_pos] = '\0';
                    shell_process_command(input_line);
                    input_pos = 0;
                    memset(input_line, 0, 256);
                }
                serial_puts("ArgOS> ");
            } else if (c == '\b' || c == 127) {
                if (input_pos > 0) {
                    input_pos--;
                    input_line[input_pos] = '\0';
                    serial_puts("\b \b");
                }
            } else if (c >= 32 && c < 127) {
                if (input_pos < 255) {
                    input_line[input_pos++] = c;
                    serial_putchar(c);
                }
            }
        }
        
        asm volatile("pause");
    }
}
