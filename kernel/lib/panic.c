#include "panic.h"
#include "graphics.h"
#include "serial.h"
#include "vga.h"
#include "string.h"

void kpanic(const char* message, const char* file, uint32_t line) {
    // 1. Log to serial (always reliable)
    serial_puts("\n!!! KERNEL PANIC !!!\n");
    serial_puts("Message: ");
    serial_puts(message);
    serial_puts("\nFile: ");
    serial_puts(file);
    serial_puts("\nLine: ");
    // Manual number to string for simplicity
    char line_buf[16];
    int i = 0;
    uint32_t n = line;
    if (n == 0) line_buf[i++] = '0';
    while (n > 0) {
        line_buf[i++] = (n % 10) + '0';
        n /= 10;
    }
    line_buf[i] = '\0';
    // Reverse
    for (int j = 0; j < i / 2; j++) {
        char temp = line_buf[j];
        line_buf[j] = line_buf[i - j - 1];
        line_buf[i - j - 1] = temp;
    }
    serial_puts(line_buf);
    serial_puts("\nSystem Halted.\n");

    // 2. Visual Panic Screen
    int sw = graphics_get_width();
    int sh = graphics_get_height();

    if (sw > 0 && sh > 0) {
        // Dark Red background
        graphics_clear(0x880000);
        
        int x = 50;
        int y = 50;
        
        graphics_draw_string(x, y, "CRITICAL KERNEL ERROR", COLOR_WHITE);
        y += 40;
        graphics_draw_rect(x, y, sw - 100, 2, COLOR_WHITE);
        y += 40;
        
        graphics_draw_string(x, y, "An unrecoverable error has occurred.", COLOR_WHITE);
        y += 30;
        graphics_draw_string(x, y, "The system has been halted to prevent damage.", COLOR_WHITE);
        
        y += 60;
        graphics_draw_string(x, y, "REASON:", COLOR_YELLOW);
        graphics_draw_string(x + 80, y, message, COLOR_WHITE);
        
        y += 30;
        graphics_draw_string(x, y, "LOCATION:", COLOR_YELLOW);
        graphics_draw_string(x + 80, y, file, COLOR_WHITE);
        graphics_draw_string(x + 80 + strlen(file) * 8 + 10, y, ":", COLOR_WHITE);
        graphics_draw_string(x + 80 + strlen(file) * 8 + 20, y, line_buf, COLOR_WHITE);
        
        y += 100;
        graphics_draw_string(x, y, "Please restart your computer.", COLOR_CYAN);
    } else {
        // Fallback to VGA text mode if graphics not initialized
        vga_clear();
        vga_puts("!!! KERNEL PANIC !!!\n");
        vga_puts(message);
    }

    // 3. Halt the CPU
    asm volatile("cli");
    while (1) {
        asm volatile("hlt");
    }
}
