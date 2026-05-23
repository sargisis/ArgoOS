// ELF Executable Loader
// Based on OSDev wiki recommendations

#include "elf.h"
#include "fs.h"
#include "pmm.h"
#include "paging.h"
#include "heap.h"
#include "string.h"
#include "serial.h"
#include "task.h"
#include "vga.h"

static int validate_elf_header(struct elf32_ehdr* ehdr) {
    // Check magic number
    if (*(uint32_t*)ehdr->e_ident != ELF_MAGIC) {
        return -1; // Invalid ELF magic
    }
    
    // Check class (must be 32-bit, e_ident[4] = 1)
    if (ehdr->e_ident[4] != 1) {
        return -1; // Not 32-bit
    }
    
    // Check endianness (little endian, e_ident[5] = 1)
    if (ehdr->e_ident[5] != 1) {
        return -1; // Not little endian
    }
    
    // Check version
    if (ehdr->e_version != 1) {
        return -1; // Invalid version
    }
    
    // Check machine type (must be i386)
    if (ehdr->e_machine != EM_386) {
        return -1; // Not i386
    }
    
    // Check file type (must be executable)
    if (ehdr->e_type != ET_EXEC) {
        return -1; // Not an executable
    }
    
    return 0; // Valid
}

int elf_load(const char* path) {
    // Open file
    int fd = fs_open(path, FS_MODE_READ);
    if (fd < 0) {
        serial_puts("elf_load: Cannot open file\n");
        return -1;
    }
    
    // Read ELF header
    struct elf32_ehdr ehdr;
    int32_t bytes_read = fs_read(fd, &ehdr, sizeof(ehdr));
    if (bytes_read < 0 || (uint32_t)bytes_read != sizeof(ehdr)) {
        serial_puts("elf_load: Cannot read ELF header\n");
        fs_close(fd);
        return -1;
    }
    
    // Validate ELF header
    if (validate_elf_header(&ehdr) < 0) {
        serial_puts("elf_load: Invalid ELF file\n");
        fs_close(fd);
        return -1;
    }
    
    serial_puts("elf_load: Valid ELF file found\n");
    serial_puts("elf_load: Entry point: 0x");
    serial_puthex(ehdr.e_entry);
    serial_puts("\n");
    
    // Read program headers
    uint32_t phdr_size = ehdr.e_phnum * ehdr.e_phentsize;
    struct elf32_phdr* phdrs = (struct elf32_phdr*)kmalloc(phdr_size);
    if (!phdrs) {
        serial_puts("elf_load: Out of memory for program headers\n");
        fs_close(fd);
        return -1;
    }
    
    // Seek to program header table
    fs_seek(fd, ehdr.e_phoff, 0); // SEEK_SET = 0
    
    // Read program headers
    bytes_read = fs_read(fd, phdrs, phdr_size);
    if (bytes_read < 0 || (uint32_t)bytes_read != phdr_size) {
        serial_puts("elf_load: Cannot read program headers\n");
        kfree(phdrs);
        fs_close(fd);
        return -1;
    }
    
    // Load each PT_LOAD segment
    for (uint16_t i = 0; i < ehdr.e_phnum; i++) {
        struct elf32_phdr* phdr = &phdrs[i];
        
        if (phdr->p_type != PT_LOAD) {
            continue; // Skip non-loadable segments
        }
        
        // Allocate memory for segment
        // For now, we'll use identity mapping (simplified)
        // In a real OS, we'd set up proper virtual memory mapping
        
        // Calculate number of pages needed
        uint32_t pages = (phdr->p_memsz + 4095) / 4096;
        
        // Allocate and map pages
        for (uint32_t j = 0; j < pages; j++) {
            void* page = pmm_alloc_page();
            if (!page) {
                serial_puts("elf_load: Out of memory\n");
                kfree(phdrs);
                fs_close(fd);
                return -1;
            }
            
            // Map page
            uint32_t vaddr = phdr->p_vaddr + (j * 4096);
            paging_map_page(vaddr, (uint32_t)page, PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER);
        }
        
        // Read segment data from file
        fs_seek(fd, phdr->p_offset, 0);
        void* segment_data = (void*)phdr->p_vaddr;
        bytes_read = fs_read(fd, segment_data, phdr->p_filesz);
        
        if (bytes_read < 0 || (uint32_t)bytes_read != phdr->p_filesz) {
            serial_puts("elf_load: Error reading segment\n");
            kfree(phdrs);
            fs_close(fd);
            return -1;
        }
        
        // Zero out .bss section (memory beyond file size)
        if (phdr->p_memsz > phdr->p_filesz) {
            memset((char*)segment_data + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
        }
    }
    
    kfree(phdrs);
    fs_close(fd);
    
    // Get current task and set entry point
    struct task* current = task_get_current();
    if (current) {
        // Set new entry point
        current->eip = ehdr.e_entry;
        
        // Set up new stack (simplified - use existing stack for now)
        // In a real OS, we'd allocate a new user stack
        
        serial_puts("elf_load: Program loaded, entry point: 0x");
        serial_puthex(ehdr.e_entry);
        serial_puts("\n");
    }
    
    return -1; // Should not reach here
}
