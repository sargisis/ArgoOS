# FlowDay-OS

A minimalistic operating system written from scratch in C and x86 assembly.

## Features

- ✅ **Multiboot 1** compatibility
- ✅ **Freestanding mode** (no standard library)
- ✅ **Custom implementations** of basic functions (string, VGA)
- ✅ **Assembly-optimized** string functions (memcpy, memset)
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
│   │   ├── string.c       # String functions
│   │   └── string_asm.asm # Assembly-optimized string functions
│   ├── memory/            # Memory management
│   │   ├── pmm.c          # Physical Memory Manager
│   │   ├── paging.c       # Virtual memory (paging)
│   │   └── heap.c         # Heap allocator
│   └── interrupts/        # Interrupt handling
│       ├── idt.asm        # IDT assembly handlers
│       ├── idt.c          # IDT implementation
│       └── pic.c          # PIC driver
│   └── drivers/           # Device drivers
│       ├── timer.c        # Timer driver (PIT)
│       ├── keyboard.c     # Keyboard driver (PS/2)
│       ├── serial.c       # Serial port driver (COM1)
│       └── ata.c          # ATA disk driver
│   └── task/              # Task/Process management
│       ├── task.c         # Task management
│       ├── context_switch.asm  # Context switching
│       ├── syscall.c      # System calls
│       └── syscall.asm    # System call entry
│   └── fs/                # File system
│       ├── fs.c           # File system implementation
│       └── elf_loader.c   # ELF executable loader
│   └── shell/             # Shell (Command Line Interface)
│       └── shell.c        # Shell implementation
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
│   ├── pic.h
│   ├── timer.h
│   ├── keyboard.h
│   ├── task.h
│   ├── syscall.h
│   ├── ata.h
│   ├── fs.h
│   ├── shell.h
│   ├── serial.h
│   └── elf.h
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
- [x] Assembly-optimized string functions
  - [x] memcpy_asm - Optimized memory copy (REP MOVSD)
  - [x] memset_asm - Optimized memory set (REP STOSD)
  - [x] 4-byte aligned copying for better performance
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
- [x] Device Drivers
  - [x] Timer (PIT) - Programmable Interval Timer
    - Configurable frequency (default 100Hz)
    - Tick counter and millisecond timing
    - Sleep function
    - Timer callbacks
  - [x] Keyboard (PS/2) - PS/2 keyboard driver
    - Scancode to ASCII conversion
    - Shift and Caps Lock support
    - Keyboard callbacks
    - Real-time key input handling
  - [x] Serial Port (COM1) - UART driver
    - Serial port initialization
    - Character I/O (putchar/getchar)
    - String output (puts)
    - Number output (putdec/puthex)
    - Used for terminal I/O in QEMU
- [x] Multitasking
  - [x] Task/Process structure
  - [x] Round-robin scheduler
  - [x] Context switching (assembly)
  - [x] System calls (INT 0x80)
    - [x] SYS_EXIT - Exit current process
    - [x] SYS_YIELD - Yield CPU to next task
    - [x] SYS_SLEEP - Sleep for specified milliseconds
    - [x] SYS_WRITE - Write to file descriptor (stdout/stderr/files)
    - [x] SYS_READ - Read from file descriptor (stdin/files)
    - [x] SYS_FORK - Clone current process
    - [x] SYS_EXEC - Load and execute ELF program
  - [x] Preemptive multitasking (timer-based)
- [x] File System
  - [x] ATA disk driver
    - [x] Read/write sectors
    - [x] Device identification
  - [x] File system interface
    - [x] File descriptors
    - [x] Open/close files
    - [x] Read/write operations
    - [x] Seek functionality
    - [x] Directory operations (list, create, remove)
    - [x] In-memory file system (files and directories)
    - [x] Path resolution (absolute/relative, ".", "..")
    - [x] Current directory management
    - [ ] Actual disk-based FS (in-memory for now)
- [x] Shell (Command Line Interface)
  - [x] Interactive command processor
  - [x] Command parsing and execution
  - [x] Built-in commands:
    - [x] `help` - Show available commands
    - [x] `clear` - Clear the screen
    - [x] `echo` - Echo arguments (with `>` redirection)
    - [x] `time` - Show system uptime
    - [x] `meminfo` - Show memory information
    - [x] `reboot` - Reboot the system
    - [x] `ls` - List directory contents
    - [x] `cat` - Display file contents
    - [x] `mkdir` - Create directory
    - [x] `rm` - Remove file or directory
    - [x] `pwd` - Print working directory
    - [x] `cd` - Change directory
    - [x] `touch` - Create empty file
    - [x] `mv` - Move or rename file
    - [x] `cp` - Copy file
    - [x] `find` - Find files by name
    - [x] `grep` - Search text in files
    - [x] `wc` - Word and line count
    - [x] `head` - Show first lines of file
    - [x] `tail` - Show last lines of file
  - [x] Command registration system
  - [x] Input handling with backspace support
  - [x] Serial port I/O support

### 🚧 In Progress / Planned

- [ ] VGA graphics mode (optional)
- [ ] Complete file system implementation
  - [ ] Directory structure on disk
  - [ ] File metadata storage
  - [ ] FAT or custom FS format
  - [ ] Persistent storage (save to disk)
- [ ] ELF Loader improvements
  - [ ] Relocations support
  - [ ] Dynamic linking
  - [ ] Shared libraries
- [ ] Process management improvements
  - [ ] Process priorities
  - [ ] Inter-process communication (IPC)
  - [ ] Synchronization primitives (semaphores, mutexes)
- [ ] Additional shell commands
  - [ ] Recursive directory operations
  - [ ] `sort` - Sort file lines
  - [ ] `uniq` - Remove duplicate lines

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
4. Run `make qemu` to test in QEMU (uses serial port for I/O)

### Running

The OS uses serial port for input/output. When you run `make qemu`, you'll see:
- All output in the terminal (via serial port)
- You can type commands directly in the terminal
- No need to grab keyboard input in QEMU window

### Available Commands

- `help` - Show all available commands
- `ls [path]` - List directory contents
- `cat <file>` - Display file contents
- `mkdir <dir>` - Create directory
- `rm <file|dir>` - Remove file or directory
- `pwd` - Print current directory
- `cd [path]` - Change directory
- `touch <file>` - Create empty file
- `echo [text] [> file]` - Echo text or write to file
- `mv <src> <dst>` - Move or rename file
- `cp <src> <dst>` - Copy file
- `find <path> <pattern>` - Find files by name
- `grep <pattern> <file>` - Search text in file
- `wc <file>` - Count lines, words, and characters
- `head <file> [lines]` - Show first lines (default: 10)
- `tail <file> [lines]` - Show last lines (default: 10)
- `time` - Show system uptime
- `meminfo` - Show memory information
- `clear` - Clear the screen
- `reboot` - Reboot the system

### Debugging

- Use QEMU with GDB: `qemu-system-i386 -kernel kernel.bin -s -S`
- Connect GDB: `gdb kernel.bin` then `target remote :1234`

## License

MIT License

## Author

FlowDay-OS Development Team

