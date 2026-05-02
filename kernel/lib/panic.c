#include "panic.h"
#include "graphics.h"
#include "serial.h"
#include "vga.h"
#include "string.h"

void kpanic(const char* message, const char* file, uint32_t line) {
    serial_puts("\n!!! KERNEL PANIC !!!\n");
    serial_puts(message);
    serial_puts("\nAt: ");
    serial_puts(file);
    serial_puts(":");
    // Print line number (quick hack)
    char line_buf[16];
    int i = 0;
    uint32_t n = line;
    if (n == 0) line_buf[i++] = '0';
    while (n > 0) { line_buf[i++] = (n % 10) + '0'; n /= 10; }
    for (int j = 0; j < i / 2; j++) { char t = line_buf[j]; line_buf[j] = line_buf[i-j-1]; line_buf[i-j-1] = t; }
    line_buf[i] = '\0';
    serial_puts(line_buf);
    serial_puts("\nSystem Halted.\n");

    int sw = graphics_get_width();
    int sh = graphics_get_height();
    if (sw > 0 && sh > 0) {
        graphics_clear(0x880000);
        graphics_draw_string(50, 50, "KERNEL PANIC", COLOR_WHITE);
        graphics_draw_string(50, 90, message, COLOR_YELLOW);
        graphics_draw_string(50, 130, file, COLOR_WHITE);
    } else {
        vga_clear();
        vga_puts("!!! KERNEL PANIC !!!\n");
        vga_puts(message);
    }
    asm volatile("cli");
    while (1) { asm volatile("hlt"); }
}
