// Multiboot структуры и константы

#ifndef MULTIBOOT_H
#define MULTIBOOT_H

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

// Multiboot information structure
struct multiboot_info {
    unsigned long flags;
    unsigned long mem_lower;
    unsigned long mem_upper;
    unsigned long boot_device;
    unsigned long cmdline;
    unsigned long mods_count;
    unsigned long mods_addr;
    unsigned long syms[4];
    unsigned long mmap_length;
    unsigned long mmap_addr;
    unsigned long drives_length;
    unsigned long drives_addr;
    unsigned long config_table;
    unsigned long boot_loader_name;
    unsigned long apm_table;
    unsigned long vbe_control_info;
    unsigned long vbe_mode_info;
    unsigned long vbe_mode;
    unsigned long vbe_interface_seg;
    unsigned long vbe_interface_off;
    unsigned long vbe_interface_len;
} __attribute__((packed));

// Memory map entry
struct multiboot_mmap_entry {
    unsigned long size;
    unsigned long addr_low;
    unsigned long addr_high;
    unsigned long len_low;
    unsigned long len_high;
    unsigned long type;
} __attribute__((packed));

#endif // MULTIBOOT_H
