#ifndef GDT_H
#define GDT_H

#include "types.h"

// GDT Entry structure
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

// GDT Pointer structure
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// GDT Constants
#define GDT_ENTRIES 5

// Access byte flags
#define GDT_ACCESS_PRESENT     0x80
#define GDT_ACCESS_RING0       0x00
#define GDT_ACCESS_RING3       0x60
#define GDT_ACCESS_CODE        0x18
#define GDT_ACCESS_DATA        0x12
#define GDT_ACCESS_EXECUTABLE  0x08
#define GDT_ACCESS_DIRECTION   0x04
#define GDT_ACCESS_READWRITE   0x02
#define GDT_ACCESS_ACCESSED    0x01

// Granularity byte flags
#define GDT_GRAN_4K            0x80
#define GDT_GRAN_32BIT         0x40

// Initialization function
void gdt_init(void);

#endif // GDT_H
