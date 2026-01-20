// VGA Text Mode Driver Header

#ifndef VGA_H
#define VGA_H

#include "types.h"

// VGA цвета
#define VGA_COLOR_BLACK         0
#define VGA_COLOR_BLUE          1
#define VGA_COLOR_GREEN         2
#define VGA_COLOR_CYAN          3
#define VGA_COLOR_RED           4
#define VGA_COLOR_MAGENTA       5
#define VGA_COLOR_BROWN         6
#define VGA_COLOR_LIGHT_GREY    7
#define VGA_COLOR_DARK_GREY     8
#define VGA_COLOR_LIGHT_BLUE    9
#define VGA_COLOR_LIGHT_GREEN   10
#define VGA_COLOR_LIGHT_CYAN    11
#define VGA_COLOR_LIGHT_RED     12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_YELLOW        14
#define VGA_COLOR_WHITE         15

// Функции VGA
void vga_init(void);
void vga_set_color(uint8_t fg, uint8_t bg);
void vga_clear(void);
void vga_putchar(char c);
void vga_puts(const char* str);
void vga_putdec(unsigned int num);
void vga_puthex(unsigned int num);

#endif // VGA_H
