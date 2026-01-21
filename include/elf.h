// ELF File Format Support
// Based on OSDev wiki recommendations

#ifndef ELF_H
#define ELF_H

#include "types.h"

// ELF Magic number
#define ELF_MAGIC 0x464C457F  // "\x7FELF"

// ELF file types
#define ET_NONE   0
#define ET_REL    1
#define ET_EXEC   2
#define ET_DYN    3
#define ET_CORE   4

// ELF machine types
#define EM_386    3  // Intel 80386

// Program header types
#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4

// Program header flags
#define PF_X 0x1  // Executable
#define PF_W 0x2  // Writable
#define PF_R 0x4  // Readable

// ELF Header (32-bit)
struct elf32_ehdr {
    uint8_t  e_ident[16];    // ELF identification
    uint16_t e_type;         // Object file type
    uint16_t e_machine;      // Machine type
    uint32_t e_version;      // Object file version
    uint32_t e_entry;        // Entry point address
    uint32_t e_phoff;        // Program header table offset
    uint32_t e_shoff;        // Section header table offset
    uint32_t e_flags;        // Processor-specific flags
    uint16_t e_ehsize;       // ELF header size
    uint16_t e_phentsize;    // Program header entry size
    uint16_t e_phnum;        // Number of program header entries
    uint16_t e_shentsize;    // Section header entry size
    uint16_t e_shnum;        // Number of section header entries
    uint16_t e_shstrndx;     // Section header string table index
};

// Program Header (32-bit)
struct elf32_phdr {
    uint32_t p_type;    // Segment type
    uint32_t p_offset;  // Segment file offset
    uint32_t p_vaddr;   // Segment virtual address
    uint32_t p_paddr;   // Segment physical address
    uint32_t p_filesz;  // Segment size in file
    uint32_t p_memsz;   // Segment size in memory
    uint32_t p_flags;   // Segment flags
    uint32_t p_align;   // Segment alignment
};

// Load ELF executable
int elf_load(const char* path);

#endif // ELF_H
