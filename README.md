# FlowDay-OS

A minimalistic operating system written from scratch in C and x86 assembly.

## Features

- ✅ **Multiboot 1** compatibility
- ✅ **Freestanding mode** (no standard library)
- ✅ **Custom implementations** of basic functions (string, VGA)
- ✅ **Memory Management**
  - Physical Memory Manager (PMM) with bitmap allocation
  - Virtual memory with paging (4KB pages)
  - Heap allocator (kmalloc/kfree/krealloc)
- ✅ **Interrupts & Exceptions**
  - IDT (Interrupt Descriptor Table) with 256 entries
  - Exception handlers for all x86 exceptions
  - IRQ handlers (0-15)
  - PIC (Programmable Interrupt Controller) driver
- ✅ Minimalism and speed
- ✅ Deep hardware understanding

## Requirements

- `nasm` - assembler
- `gcc` - C compiler (with 32-bit support)
- `ld` - linker
- `grub-mkrescue` - for creating ISO image
- `qemu-system-i386` - for emulation

### Installing Dependencies (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install nasm gcc-multilib binutils grub-pc-bin qemu-system-x86
```

## Building

```bash
make
```

This will create `kernel.bin` - the compiled kernel.

## Running

### Option 1: Direct kernel loading via QEMU

```bash
make qemu
```

### Option 2: ISO image (recommended)

```bash
make iso
make run
```

## Project Structure

```
FlowDay-OS/
├── boot/                  # Multiboot entry point
│   └── multiboot.asm
├── kernel/                # Main kernel
│   ├── kernel.c           # Main kernel file
│   ├── lib/               # Library functions
│   │   ├── vga.c          # VGA driver
│   │   └── string.c       # String functions
│   ├── memory/            # Memory management
│   │   ├── pmm.c          # Physical Memory Manager
│   │   ├── paging.c       # Virtual memory (paging)
│   │   └── heap.c         # Heap allocator
│   └── interrupts/        # Interrupt handling
│       ├── idt.asm        # IDT assembly handlers
│       ├── idt.c          # IDT implementation
│       └── pic.c          # PIC driver
├── include/               # Header files
│   ├── kernel.h
│   ├── multiboot.h
│   ├── vga.h
│   ├── string.h
│   ├── types.h
│   ├── pmm.h
│   ├── paging.h
│   ├── heap.h
│   ├── idt.h
│   └── pic.h
├── kernel.ld              # Linker script
├── grub.cfg               # GRUB configuration
└── Makefile               # Build system
```

## Current Status

### ✅ Completed

- [x] Multiboot bootloader
- [x] Basic kernel initialization
- [x] VGA text mode driver
- [x] Custom string functions (strlen, memcpy, memset, etc.)
- [x] Physical Memory Manager (PMM)
  - Bitmap-based page allocation
  - Memory map parsing from Multiboot
  - Page allocation/deallocation
- [x] Virtual Memory (Paging)
  - Page Directory and Page Tables
  - Identity mapping for first 4MB
  - Dynamic page mapping
- [x] Heap Allocator
  - kmalloc/kfree/krealloc
  - Linked list-based block management
  - Automatic heap expansion
- [x] Interrupt Descriptor Table (IDT)
  - 32 exception handlers
  - 16 IRQ handlers
  - Exception error reporting
- [x] PIC (Programmable Interrupt Controller)
  - Master and slave PIC initialization
  - IRQ masking/unmasking
  - EOI (End Of Interrupt) handling

### 🚧 In Progress / Planned

- [ ] Device Drivers
  - [ ] Timer (PIT) - for multitasking
  - [ ] Keyboard (PS/2) - for input
  - [ ] VGA graphics mode (optional)
- [ ] Multitasking
  - [ ] Process scheduler
  - [ ] Context switching
  - [ ] System calls (syscalls)
- [ ] File System
  - [ ] Disk driver (ATA)
  - [ ] Simple FS or FAT support
- [ ] Shell
  - [ ] Command line interface
  - [ ] Basic commands (ls, cd, cat, etc.)

## Architecture

- **Architecture**: x86 (32-bit)
- **Boot**: Multiboot 1 specification
- **Memory Model**: Protected mode with paging
- **Code Style**: Freestanding C (no stdlib)

## Memory Layout

- `0x00000000 - 0x000FFFFF`: First 1MB (kernel, boot code)
- `0x00100000 - 0x001FFFFF`: Kernel code (1MB)
- `0x00200000 - 0x003FFFFF`: PMM bitmap and structures
- `0x00400000+`: Heap area (dynamic allocation)

## Interrupts

- **0-31**: CPU Exceptions (Division by Zero, Page Fault, etc.)
- **32-47**: IRQ handlers (Timer, Keyboard, etc.)
- **48+**: Available for system calls

## Development

### Code Style

- Freestanding C (no standard library)
- All basic functions implemented from scratch
- Minimal dependencies
- Clear, commented code

### Building from Source

1. Clone the repository
2. Install dependencies (see Requirements)
3. Run `make` to build
4. Run `make qemu` to test in QEMU

### Debugging

- Use QEMU with GDB: `qemu-system-i386 -kernel kernel.bin -s -S`
- Connect GDB: `gdb kernel.bin` then `target remote :1234`

## License

MIT License

## Author

FlowDay-OS Development Team

## Acknowledgments

- Inspired by OSDev.org tutorials
- Multiboot specification
- x86 architecture documentation
