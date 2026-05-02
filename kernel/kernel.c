// ArgOS Kernel
// Основной файл ядра

#include "kernel.h"
#include "multiboot.h"
#include "gdt.h"
#include "cpu.h"
#include "panic.h"
#include "printf.h"
#include "string.h"
#include "pmm.h"
#include "paging.h"
#include "heap.h"
#include "idt.h"
#include "pic.h"
#include "vga.h"
#include "keyboard.h"
#include "serial.h"
#include "task.h"
#include "syscall.h"
#include "ata.h"
#include "fs.h"
#include "shell.h"
#include "graphics.h"
#include "timer.h"

// Forward declaration
extern char keyboard_scancode_to_ascii(uint8_t scancode);

// Глобальные переменные
struct multiboot_info* mb_info = 0;

void kernel_main(unsigned long magic, unsigned long addr) {
    // 1. Сразу инициализируем базовый вывод (VGA и Serial) для логов
    vga_init();
    vga_clear();
    serial_init();

    // 2. Проверка Multiboot magic number
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        kprintf("ERROR: Invalid Multiboot magic number: 0x%x\n", magic);
        PANIC("Invalid Multiboot magic number");
    }
    
    // Сохраняем указатель на multiboot info
    mb_info = (struct multiboot_info*)addr;
    
    // 3. Инициализация архитектуры CPU (GDT, FPU)
    gdt_init();
    cpu_init_fpu();
    
    // 4. Инициализация управления памятью
    kprintf("Initializing memory management...\n");
    pmm_init(mb_info);
    paging_init();
    heap_init();
    
    // 5. Инициализация прерываний
    kprintf("Initializing interrupts (IDT/PIC)...\n");
    idt_init();
    pic_init();
    
    // 6. Инициализация драйверов устройств
    kprintf("Initializing device drivers...\n");
    timer_init(TIMER_FREQUENCY);
    kprintf("Timer initialized at %d Hz\n", TIMER_FREQUENCY);
    keyboard_init();
    kprintf("Keyboard driver ready\n");
    
    // 7. Инициализация графики (ПОСЛЕ paging!)
    graphics_init(mb_info);
    kprintf("Graphics system initialized\n");
    
    // 8. Инициализация многозадачности
    kprintf("Initializing multitasking...\n");
    task_init();
    syscall_init();
    kprintf("Multitasking ready (Round-robin)\n");
    
    // 9. Инициализация файловой системы
    kprintf("Initializing file system...\n");
    ata_init();
    fs_init();
    kprintf("File system ready\n");
    
    // 10. Включаем прерывания
    asm volatile("sti");
    kprintf("Interrupts enabled. Kernel is fully operational.\n");
    
    // 11. Рисуем графический интерфейс (GUI)
    int sw = graphics_get_width();
    int sh = graphics_get_height();
    
    if (sw > 0 && sh > 0) {
        // Обои рабочего стола (Темный космос)
        graphics_clear(COLOR_DARK_BG);
        
        // Панель задач снизу (Taskbar)
        graphics_draw_rect(0, sh - 40, sw, 40, 0x111111);
        
        // Кнопка "Пуск" (ArgOS Logo)
        graphics_draw_rect(10, sh - 35, 60, 30, COLOR_CYAN);
        graphics_draw_string(18, sh - 30, "ArgOS", 0x000000);
        
        // Часы на панели задач
        graphics_draw_string(sw - 60, sh - 28, "19:47", COLOR_WHITE);
        
        // Главное окно
        int win_w = 400;
        int win_h = 300;
        int win_x = (sw - win_w) / 2;
        int win_y = (sh - win_h) / 2;
        
        graphics_draw_rect(win_x, win_y, win_w, win_h, 0x222233);
        graphics_draw_rect(win_x, win_y, win_w, 25, 0x333355);
        graphics_draw_string(win_x + 8, win_y + 5, "Welcome to ArgOS", COLOR_WHITE);
        
        graphics_draw_string(win_x + 20, win_y + 50, "ArgOS Kernel v0.1", COLOR_CYAN);
        graphics_draw_string(win_x + 20, win_y + 80, "Graphics: 800x600 32-bit", COLOR_GREEN);
        graphics_draw_string(win_x + 20, win_y + 110, "FPU/SSE: Enabled", COLOR_GREEN);
        
        kprintf("GUI rendered successfully\n");
    }
    
    // Shell через serial port
    kprintf("\n=== ArgOS Shell (Serial) ===\n");
    shell_init();
    
    char input_line[256] = {0};
    int input_pos = 0;
    
    kprintf("ArgOS> ");
    
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
                kprintf("ArgOS> ");
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
