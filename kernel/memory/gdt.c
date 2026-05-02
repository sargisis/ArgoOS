#include "gdt.h"

static struct gdt_entry gdt[GDT_ENTRIES];
static struct gdt_ptr gp;

// External assembly function to load GDT
extern void gdt_flush(uint32_t gp_addr);

static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;

    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access = access;
}

void gdt_init(void) {
    gp.limit = (sizeof(struct gdt_entry) * GDT_ENTRIES) - 1;
    gp.base = (uint32_t)&gdt;

    // 0x00: Null segment
    gdt_set_gate(0, 0, 0, 0, 0);

    // 0x08: Kernel Code segment (Base: 0, Limit: 4GB, Type: Code, Ring 0)
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // 0x10: Kernel Data segment (Base: 0, Limit: 4GB, Type: Data, Ring 0)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // 0x18: User Code segment (Base: 0, Limit: 4GB, Type: Code, Ring 3)
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    // 0x20: User Data segment (Base: 0, Limit: 4GB, Type: Data, Ring 3)
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    // Load the GDT
    gdt_flush((uint32_t)&gp);
}
