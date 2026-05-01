#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "types.h"
#include "multiboot.h"

// Basic Colors (RGB 32-bit)
#define COLOR_BLACK    0x000000
#define COLOR_WHITE    0xFFFFFF
#define COLOR_RED      0xFF0000
#define COLOR_GREEN    0x00FF00
#define COLOR_BLUE     0x0000FF
#define COLOR_YELLOW   0xFFFF00
#define COLOR_CYAN     0x00FFFF
#define COLOR_MAGENTA  0xFF00FF
#define COLOR_GRAY     0x808080
#define COLOR_DARK_BG  0x1A1A2E // Deep space theme

void graphics_init(struct multiboot_info* mb_info);
void graphics_put_pixel(int x, int y, uint32_t color);
void graphics_draw_rect(int x, int y, int width, int height, uint32_t color);
void graphics_clear(uint32_t color);
int graphics_get_width(void);
int graphics_get_height(void);

#endif // GRAPHICS_H
