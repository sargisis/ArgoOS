// VGA Text Mode Driver
// Работа с VGA текстовым режимом (0xB8000)

#include "vga.h"

// VGA буфер начинается с адреса 0xB8000
static volatile char* const VGA_MEMORY = (volatile char*)0xB8000;
static const int VGA_WIDTH = 80;
static const int VGA_HEIGHT = 25;

// Текущая позиция курсора
static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t current_color = VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4);

void vga_init(void) {
    cursor_x = 0;
    cursor_y = 0;
    current_color = VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4);
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    current_color = fg | (bg << 4);
}

void vga_clear(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            const int index = (y * VGA_WIDTH + x) * 2;
            VGA_MEMORY[index] = ' ';
            VGA_MEMORY[index + 1] = current_color;
        }
    }
    cursor_x = 0;
    cursor_y = 0;
}

void vga_scroll(void) {
    // Копируем все строки на одну вверх
    for (int y = 1; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            const int src_index = (y * VGA_WIDTH + x) * 2;
            const int dst_index = ((y - 1) * VGA_WIDTH + x) * 2;
            VGA_MEMORY[dst_index] = VGA_MEMORY[src_index];
            VGA_MEMORY[dst_index + 1] = VGA_MEMORY[src_index + 1];
        }
    }
    
    // Очищаем последнюю строку
    for (int x = 0; x < VGA_WIDTH; x++) {
        const int index = ((VGA_HEIGHT - 1) * VGA_WIDTH + x) * 2;
        VGA_MEMORY[index] = ' ';
        VGA_MEMORY[index + 1] = current_color;
    }
    
    cursor_y = VGA_HEIGHT - 1;
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\t') {
        cursor_x = (cursor_x + 4) & ~(4 - 1);
    } else {
        const int index = (cursor_y * VGA_WIDTH + cursor_x) * 2;
        VGA_MEMORY[index] = c;
        VGA_MEMORY[index + 1] = current_color;
        cursor_x++;
    }
    
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }
    
    if (cursor_y >= VGA_HEIGHT) {
        vga_scroll();
    }
}

void vga_puts(const char* str) {
    while (*str) {
        vga_putchar(*str++);
    }
}

void vga_putdec(unsigned int num) {
    if (num == 0) {
        vga_putchar('0');
        return;
    }
    
    char buffer[32];
    int i = 0;
    
    while (num > 0) {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    
    // Выводим в обратном порядке
    for (int j = i - 1; j >= 0; j--) {
        vga_putchar(buffer[j]);
    }
}

void vga_puthex(unsigned int num) {
    vga_puts("0x");
    
    if (num == 0) {
        vga_putchar('0');
        return;
    }
    
    char buffer[32];
    int i = 0;
    
    while (num > 0) {
        int digit = num % 16;
        buffer[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        num /= 16;
    }
    
    // Выводим в обратном порядке
    for (int j = i - 1; j >= 0; j--) {
        vga_putchar(buffer[j]);
    }
}
