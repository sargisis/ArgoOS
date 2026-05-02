# Makefile для FlowDay-OS
# Multiboot совместимое ядро

# Компиляторы и утилиты
ASM = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy

# Флаги компиляции
ASMFLAGS = -f elf32
CFLAGS = -m32 -ffreestanding -nostdlib -nostdinc -fno-builtin -fno-stack-protector \
         -Wall -Wextra -Werror -O0 -I./include
LDFLAGS = -m elf_i386 -T kernel.ld

# Директории
BOOT_DIR = boot
KERNEL_DIR = kernel
LIB_DIR = $(KERNEL_DIR)/lib
MEMORY_DIR = $(KERNEL_DIR)/memory
INTERRUPTS_DIR = $(KERNEL_DIR)/interrupts
DRIVERS_DIR = $(KERNEL_DIR)/drivers
TASK_DIR = $(KERNEL_DIR)/task
FS_DIR = $(KERNEL_DIR)/fs
SHELL_DIR = $(KERNEL_DIR)/shell
INCLUDE_DIR = include

# Объектные файлы
BOOT_OBJ = $(BOOT_DIR)/multiboot.o
KERNEL_OBJ = $(KERNEL_DIR)/kernel.o
LIB_OBJS = $(LIB_DIR)/vga.o $(LIB_DIR)/string.o $(LIB_DIR)/string_asm.o \
           $(LIB_DIR)/font.o $(LIB_DIR)/panic.o $(LIB_DIR)/printf.o
MEMORY_OBJS = $(MEMORY_DIR)/pmm.o $(MEMORY_DIR)/paging.o $(MEMORY_DIR)/heap.o \
              $(MEMORY_DIR)/gdt.o $(MEMORY_DIR)/gdt_asm.o
INTERRUPTS_OBJS = $(INTERRUPTS_DIR)/idt.o $(INTERRUPTS_DIR)/idt_c.o $(INTERRUPTS_DIR)/pic.o
DRIVERS_OBJS = $(DRIVERS_DIR)/timer.o $(DRIVERS_DIR)/keyboard.o $(DRIVERS_DIR)/serial.o \
                $(DRIVERS_DIR)/ata.o $(DRIVERS_DIR)/graphics.o
TASK_ASM_OBJS = $(TASK_DIR)/context_switch.o $(TASK_DIR)/syscall.o $(TASK_DIR)/task_entry.o
TASK_OBJS = $(TASK_DIR)/task.o $(TASK_DIR)/syscall_c.o $(TASK_DIR)/sync.o
FS_OBJS = $(FS_DIR)/fs.o $(FS_DIR)/elf_loader.o
SHELL_OBJS = $(SHELL_DIR)/shell.o
CPU_OBJS = $(KERNEL_DIR)/cpu/fpu.o

# Итоговый образ
KERNEL_BIN = kernel.bin
OS_IMAGE = flowday-os.iso

.PHONY: all clean run qemu iso

all: $(KERNEL_BIN)

$(BOOT_OBJ): $(BOOT_DIR)/multiboot.asm
	$(ASM) $(ASMFLAGS) $< -o $@

$(KERNEL_OBJ): $(KERNEL_DIR)/kernel.c
	$(CC) $(CFLAGS) -c $< -o $@

# LIB rules
$(LIB_DIR)/vga.o: $(LIB_DIR)/vga.c
	$(CC) $(CFLAGS) -c $< -o $@
$(LIB_DIR)/string.o: $(LIB_DIR)/string.c
	$(CC) $(CFLAGS) -c $< -o $@
$(LIB_DIR)/string_asm.o: $(LIB_DIR)/string_asm.asm
	$(ASM) $(ASMFLAGS) $< -o $@
$(LIB_DIR)/font.o: $(LIB_DIR)/font.c
	$(CC) $(CFLAGS) -c $< -o $@
$(LIB_DIR)/panic.o: $(LIB_DIR)/panic.c
	$(CC) $(CFLAGS) -c $< -o $@
$(LIB_DIR)/printf.o: $(LIB_DIR)/printf.c
	$(CC) $(CFLAGS) -c $< -o $@

# MEMORY rules
$(MEMORY_DIR)/pmm.o: $(MEMORY_DIR)/pmm.c
	$(CC) $(CFLAGS) -c $< -o $@
$(MEMORY_DIR)/paging.o: $(MEMORY_DIR)/paging.c
	$(CC) $(CFLAGS) -c $< -o $@
$(MEMORY_DIR)/heap.o: $(MEMORY_DIR)/heap.c
	$(CC) $(CFLAGS) -c $< -o $@
$(MEMORY_DIR)/gdt.o: $(MEMORY_DIR)/gdt.c
	$(CC) $(CFLAGS) -c $< -o $@
$(MEMORY_DIR)/gdt_asm.o: $(MEMORY_DIR)/gdt_asm.asm
	$(ASM) $(ASMFLAGS) $< -o $@

# INTERRUPTS rules
$(INTERRUPTS_DIR)/idt.o: $(INTERRUPTS_DIR)/idt.asm
	$(ASM) $(ASMFLAGS) $< -o $@
$(INTERRUPTS_DIR)/idt_c.o: $(INTERRUPTS_DIR)/idt.c
	$(CC) $(CFLAGS) -c $< -o $@
$(INTERRUPTS_DIR)/pic.o: $(INTERRUPTS_DIR)/pic.c
	$(CC) $(CFLAGS) -c $< -o $@

# DRIVERS rules
$(DRIVERS_DIR)/timer.o: $(DRIVERS_DIR)/timer.c
	$(CC) $(CFLAGS) -c $< -o $@
$(DRIVERS_DIR)/keyboard.o: $(DRIVERS_DIR)/keyboard.c
	$(CC) $(CFLAGS) -c $< -o $@
$(DRIVERS_DIR)/serial.o: $(DRIVERS_DIR)/serial.c
	$(CC) $(CFLAGS) -I./include -c $< -o $@
$(DRIVERS_DIR)/ata.o: $(DRIVERS_DIR)/ata.c
	$(CC) $(CFLAGS) -c $< -o $@
$(DRIVERS_DIR)/graphics.o: $(DRIVERS_DIR)/graphics.c
	$(CC) $(CFLAGS) -c $< -o $@

# TASK rules
$(TASK_DIR)/context_switch.o: $(TASK_DIR)/context_switch.asm
	$(ASM) $(ASMFLAGS) $< -o $@
$(TASK_DIR)/syscall.o: $(TASK_DIR)/syscall.asm
	$(ASM) $(ASMFLAGS) $< -o $@
$(TASK_DIR)/task_entry.o: $(TASK_DIR)/task_entry.asm
	$(ASM) $(ASMFLAGS) $< -o $@
$(TASK_DIR)/task.o: $(TASK_DIR)/task.c
	$(CC) $(CFLAGS) -c $< -o $@
$(TASK_DIR)/syscall_c.o: $(TASK_DIR)/syscall.c
	$(CC) $(CFLAGS) -c $< -o $@
$(TASK_DIR)/sync.o: $(TASK_DIR)/sync.c
	$(CC) $(CFLAGS) -c $< -o $@

# OTHER rules
$(FS_DIR)/fs.o: $(FS_DIR)/fs.c
	$(CC) $(CFLAGS) -c $< -o $@
$(FS_DIR)/elf_loader.o: $(FS_DIR)/elf_loader.c
	$(CC) $(CFLAGS) -c $< -o $@
$(SHELL_DIR)/shell.o: $(SHELL_DIR)/shell.c
	$(CC) $(CFLAGS) -c $< -o $@
$(KERNEL_DIR)/cpu/fpu.o: $(KERNEL_DIR)/cpu/fpu.c
	$(CC) $(CFLAGS) -c $< -o $@

# Линковка kernel
$(KERNEL_BIN): $(BOOT_OBJ) $(KERNEL_OBJ) $(LIB_OBJS) $(MEMORY_OBJS) $(INTERRUPTS_OBJS) $(DRIVERS_OBJS) $(TASK_ASM_OBJS) $(TASK_OBJS) $(FS_OBJS) $(SHELL_OBJS) $(CPU_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

iso: $(KERNEL_BIN)
	mkdir -p isodir/boot/grub
	cp $(KERNEL_BIN) isodir/boot/
	cp boot/grub.cfg isodir/boot/grub/ 2>/dev/null || true
	grub-mkrescue -o $(OS_IMAGE) isodir

qemu: $(KERNEL_BIN)
	qemu-system-i386 -kernel kernel.bin -vga std -serial stdio

clean:
	rm -f $(BOOT_OBJ) $(KERNEL_OBJ) $(LIB_OBJS) $(MEMORY_OBJS) $(INTERRUPTS_OBJS) $(DRIVERS_OBJS) $(TASK_ASM_OBJS) $(TASK_OBJS) $(FS_OBJS) $(SHELL_OBJS) $(CPU_OBJS)
	rm -f kernel.bin $(OS_IMAGE)
	rm -rf isodir
