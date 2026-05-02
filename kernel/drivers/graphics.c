#include "graphics.h"
#include "serial.h"
#include "paging.h"
#include "font.h"

// Bochs VGA Extensions (BGA) - works in QEMU with -vga std
#define BGA_INDEX_PORT  0x01CE
#define BGA_DATA_PORT   0x01CF

#define BGA_INDEX_ID          0x0
#define BGA_INDEX_XRES        0x1
#define BGA_INDEX_YRES        0x2
#define BGA_INDEX_BPP         0x3
#define BGA_INDEX_ENABLE      0x4

#define BGA_DISABLED    0x00
#define BGA_ENABLED     0x01
#define BGA_LFB_ENABLED 0x40

static uint32_t* framebuffer = 0;
static int screen_width = 0;
static int screen_height = 0;
static int screen_bpp = 0;

// --- PCI helpers to find the real framebuffer address ---
static void pci_outl(uint16_t port, uint32_t value) {
    asm volatile("outl %0, %1" :: "a"(value), "Nd"(port));
}

static uint32_t pci_inl(uint16_t port) {
    uint32_t val;
    asm volatile("inl %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static uint32_t pci_config_read(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((1u << 31) | ((uint32_t)bus << 16) |
                       ((uint32_t)device << 11) | ((uint32_t)func << 8) |
                       (offset & 0xFC));
    pci_outl(0xCF8, address);
    return pci_inl(0xCFC);
}

static uint32_t find_bga_framebuffer(void) {
    for (uint8_t bus = 0; bus < 8; bus++) {
        for (uint8_t dev = 0; dev < 32; dev++) {
            uint32_t id = pci_config_read(bus, dev, 0, 0);
            uint16_t vendor = id & 0xFFFF;
            uint16_t device = (id >> 16) & 0xFFFF;
            if (vendor == 0x1234 && device == 0x1111) {
                uint32_t bar0 = pci_config_read(bus, dev, 0, 0x10);
                bar0 &= ~0xFu;
                serial_puts("Found Bochs VGA BAR0=0x");
                for (int i = 28; i >= 0; i -= 4) {
                    int nibble = (bar0 >> i) & 0xF;
                    serial_putchar(nibble < 10 ? '0' + nibble : 'A' + nibble - 10);
                }
                serial_puts("\n");
                return bar0;
            }
        }
    }
    return 0xE0000000;
}

static void bga_set_mode(int w, int h, int bpp) {
    asm volatile("outw %0, %1" :: "a"((uint16_t)BGA_INDEX_ENABLE), "Nd"((uint16_t)BGA_INDEX_PORT));
    asm volatile("outw %0, %1" :: "a"((uint16_t)BGA_DISABLED), "Nd"((uint16_t)BGA_DATA_PORT));

    asm volatile("outw %0, %1" :: "a"((uint16_t)BGA_INDEX_XRES), "Nd"((uint16_t)BGA_INDEX_PORT));
    asm volatile("outw %0, %1" :: "a"((uint16_t)w), "Nd"((uint16_t)BGA_DATA_PORT));

    asm volatile("outw %0, %1" :: "a"((uint16_t)BGA_INDEX_YRES), "Nd"((uint16_t)BGA_INDEX_PORT));
    asm volatile("outw %0, %1" :: "a"((uint16_t)h), "Nd"((uint16_t)BGA_DATA_PORT));

    asm volatile("outw %0, %1" :: "a"((uint16_t)BGA_INDEX_BPP), "Nd"((uint16_t)BGA_INDEX_PORT));
    asm volatile("outw %0, %1" :: "a"((uint16_t)bpp), "Nd"((uint16_t)BGA_DATA_PORT));

    asm volatile("outw %0, %1" :: "a"((uint16_t)BGA_INDEX_ENABLE), "Nd"((uint16_t)BGA_INDEX_PORT));
    asm volatile("outw %0, %1" :: "a"((uint16_t)(BGA_ENABLED | BGA_LFB_ENABLED)), "Nd"((uint16_t)BGA_DATA_PORT));
}

void graphics_init(struct multiboot_info* mb_info) {
    if (!mb_info) {
        serial_puts("ERROR: No Multiboot info provided to graphics_init\n");
        return;
    }

    // Check if framebuffer info is available (bit 12 in flags)
    if (!(mb_info->flags & (1 << 12))) {
        serial_puts("ERROR: No framebuffer info in Multiboot structure\n");
        // Fallback to BGA if multiboot fails
        asm volatile("outw %0, %1" :: "a"((uint16_t)BGA_INDEX_ID), "Nd"((uint16_t)BGA_INDEX_PORT));
        uint16_t id;
        asm volatile("inw %1, %0" : "=a"(id) : "Nd"((uint16_t)BGA_DATA_PORT));
        if (id < 0xB0C0 || id > 0xB0C5) return;
        
        screen_width = 800;
        screen_height = 600;
        screen_bpp = 32;
        bga_set_mode(screen_width, screen_height, screen_bpp);
        framebuffer = (uint32_t*)find_bga_framebuffer();
    } else {
        // Use Multiboot framebuffer info
        framebuffer = (uint32_t*)(uint32_t)mb_info->framebuffer_addr_low;
        screen_width = mb_info->framebuffer_width;
        screen_height = mb_info->framebuffer_height;
        screen_bpp = mb_info->framebuffer_bpp;
        serial_puts("Multiboot Graphics initialized: ");
    }

    if (!framebuffer) {
        serial_puts("ERROR: Framebuffer address is NULL\n");
        return;
    }

    // Map the framebuffer into virtual memory
    uint32_t fb_size = (uint32_t)screen_width * (uint32_t)screen_height * (screen_bpp / 8);
    uint32_t num_pages = (fb_size + 0xFFF) / 0x1000;
    
    // We map the physical address to the same virtual address (Identity Mapping)
    for (uint32_t i = 0; i < num_pages; i++) {
        uint32_t addr = (uint32_t)framebuffer + (i * 0x1000);
        paging_map_page(addr, addr, PAGE_PRESENT | PAGE_WRITABLE);
    }

    // Output debug info to serial
    serial_puts("FB=0x");
    uint32_t fb_ptr = (uint32_t)framebuffer;
    for (int i = 28; i >= 0; i -= 4) {
        int nibble = (fb_ptr >> i) & 0xF;
        serial_putchar(nibble < 10 ? '0' + nibble : 'A' + nibble - 10);
    }
    serial_puts("\n");
}

void graphics_put_pixel(int x, int y, uint32_t color) {
    if (x < 0 || x >= screen_width || y < 0 || y >= screen_height || !framebuffer) return;
    framebuffer[y * screen_width + x] = color;
}

void graphics_draw_rect(int x, int y, int w, int h, uint32_t color) {
    for (int j = 0; j < h; j++) {
        int py = y + j;
        if (py < 0 || py >= screen_height) continue;
        for (int i = 0; i < w; i++) {
            int px = x + i;
            if (px < 0 || px >= screen_width) continue;
            framebuffer[py * screen_width + px] = color;
        }
    }
}

void graphics_clear(uint32_t color) {
    if (!framebuffer) return;
    int total = screen_width * screen_height;
    for (int i = 0; i < total; i++) {
        framebuffer[i] = color;
    }
}

int graphics_get_width(void) { return screen_width; }
int graphics_get_height(void) { return screen_height; }

void graphics_draw_char(int x, int y, char c, uint32_t color) {
    const uint8_t* glyph = font_get_glyph(c);
    for (int row = 0; row < FONT_HEIGHT; row++) {
        uint8_t bits = glyph[row];
        for (int col = 0; col < FONT_WIDTH; col++) {
            if (bits & (0x80 >> col)) {
                graphics_put_pixel(x + col, y + row, color);
            }
        }
    }
}

void graphics_draw_string(int x, int y, const char* str, uint32_t color) {
    while (*str) {
        graphics_draw_char(x, y, *str, color);
        x += FONT_WIDTH;
        str++;
    }
}
