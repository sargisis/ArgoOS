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
         -Wall -Wextra -Werror -O2 -I./include
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

# Исходные файлы
BOOT_SRC = $(BOOT_DIR)/multiboot.asm
KERNEL_SRC = $(KERNEL_DIR)/kernel.c
LIB_SRCS = $(LIB_DIR)/vga.c $(LIB_DIR)/string.c
LIB_ASM = $(LIB_DIR)/string_asm.asm
MEMORY_SRCS = $(MEMORY_DIR)/pmm.c $(MEMORY_DIR)/paging.c $(MEMORY_DIR)/heap.c
INTERRUPTS_ASM = $(INTERRUPTS_DIR)/idt.asm
INTERRUPTS_SRCS = $(INTERRUPTS_DIR)/idt.c $(INTERRUPTS_DIR)/pic.c
DRIVERS_SRCS = $(DRIVERS_DIR)/timer.c $(DRIVERS_DIR)/keyboard.c $(DRIVERS_DIR)/serial.c $(DRIVERS_DIR)/ata.c
TASK_ASM = $(TASK_DIR)/context_switch.asm $(TASK_DIR)/syscall.asm
TASK_SRCS = $(TASK_DIR)/task.c $(TASK_DIR)/syscall.c $(TASK_DIR)/sync.c
FS_SRCS = $(FS_DIR)/fs.c $(FS_DIR)/elf_loader.c
SHELL_SRCS = $(SHELL_DIR)/shell.c

# Объектные файлы
BOOT_OBJ = $(BOOT_DIR)/multiboot.o
KERNEL_OBJ = $(KERNEL_DIR)/kernel.o
LIB_OBJS = $(LIB_DIR)/vga.o $(LIB_DIR)/string.o $(LIB_DIR)/string_asm.o
MEMORY_OBJS = $(MEMORY_DIR)/pmm.o $(MEMORY_DIR)/paging.o $(MEMORY_DIR)/heap.o
INTERRUPTS_OBJS = $(INTERRUPTS_DIR)/idt.o $(INTERRUPTS_DIR)/idt_c.o $(INTERRUPTS_DIR)/pic.o
DRIVERS_OBJS = $(DRIVERS_DIR)/timer.o $(DRIVERS_DIR)/keyboard.o $(DRIVERS_DIR)/serial.o $(DRIVERS_DIR)/ata.o
TASK_ASM_OBJS = $(TASK_DIR)/context_switch.o $(TASK_DIR)/syscall.o $(TASK_DIR)/task_entry.o
TASK_OBJS = $(TASK_DIR)/task.o $(TASK_DIR)/syscall_c.o $(TASK_DIR)/sync.o
FS_OBJS = $(FS_DIR)/fs.o $(FS_DIR)/elf_loader.o
SHELL_OBJS = $(SHELL_DIR)/shell.o

# Итоговый образ
KERNEL_BIN = kernel.bin
OS_IMAGE = flowday-os.iso
DISK_IMG = disk.img

.PHONY: all clean run qemu qemu-disk iso disk

all: $(KERNEL_BIN)

# Сборка bootloader (Multiboot entry point)
$(BOOT_OBJ): $(BOOT_SRC)
	$(ASM) $(ASMFLAGS) $< -o $@

# Сборка kernel
$(KERNEL_OBJ): $(KERNEL_SRC)
	$(CC) $(CFLAGS) -c $< -o $@

# Сборка библиотек
$(LIB_DIR)/vga.o: $(LIB_DIR)/vga.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_DIR)/string.o: $(LIB_DIR)/string.c
	$(CC) $(CFLAGS) -c $< -o $@

# Assembly-optimized string functions
$(LIB_DIR)/string_asm.o: $(LIB_DIR)/string_asm.asm
	$(ASM) $(ASMFLAGS) $< -o $@

# Сборка memory management
$(MEMORY_DIR)/pmm.o: $(MEMORY_DIR)/pmm.c
	$(CC) $(CFLAGS) -c $< -o $@

$(MEMORY_DIR)/paging.o: $(MEMORY_DIR)/paging.c
	$(CC) $(CFLAGS) -c $< -o $@

$(MEMORY_DIR)/heap.o: $(MEMORY_DIR)/heap.c
	$(CC) $(CFLAGS) -c $< -o $@

# Сборка interrupts
$(INTERRUPTS_DIR)/idt.o: $(INTERRUPTS_ASM)
	$(ASM) $(ASMFLAGS) $< -o $@

$(INTERRUPTS_DIR)/idt_c.o: $(INTERRUPTS_DIR)/idt.c
	$(CC) $(CFLAGS) -c $< -o $@

$(INTERRUPTS_DIR)/pic.o: $(INTERRUPTS_DIR)/pic.c
	$(CC) $(CFLAGS) -c $< -o $@

# Сборка drivers
$(DRIVERS_DIR)/timer.o: $(DRIVERS_DIR)/timer.c
	$(CC) $(CFLAGS) -c $< -o $@

$(DRIVERS_DIR)/keyboard.o: $(DRIVERS_DIR)/keyboard.c
	$(CC) $(CFLAGS) -c $< -o $@

$(DRIVERS_DIR)/ata.o: $(DRIVERS_DIR)/ata.c
	$(CC) $(CFLAGS) -c $< -o $@

# Сборка task management
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

# Сборка file system
$(FS_DIR)/fs.o: $(FS_DIR)/fs.c
	$(CC) $(CFLAGS) -c $< -o $@

# Сборка shell
$(SHELL_DIR)/shell.o: $(SHELL_DIR)/shell.c
	$(CC) $(CFLAGS) -c $< -o $@

# Линковка kernel
$(KERNEL_BIN): $(BOOT_OBJ) $(KERNEL_OBJ) $(LIB_OBJS) $(MEMORY_OBJS) $(INTERRUPTS_OBJS) $(DRIVERS_OBJS) $(TASK_ASM_OBJS) $(TASK_OBJS) $(FS_OBJS) $(SHELL_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

# Создание ISO образа (для загрузки через GRUB)
iso: $(KERNEL_BIN)
	mkdir -p isodir/boot/grub
	cp $(KERNEL_BIN) isodir/boot/
	cp grub.cfg isodir/boot/grub/
	grub-mkrescue -o $(OS_IMAGE) isodir

# Запуск в QEMU (с ISO)
run: iso
	qemu-system-i386 -cdrom $(OS_IMAGE)

# Запуск в QEMU (прямая загрузка kernel с serial port)
qemu: $(KERNEL_BIN)
	qemu-system-i386 -kernel $(KERNEL_BIN) -serial stdio -display none

# Создание диска для файловой системы
disk: $(DISK_IMG)

$(DISK_IMG):
	dd if=/dev/zero of=$(DISK_IMG) bs=1M count=10 2>/dev/null || \
	qemu-img create -f raw $(DISK_IMG) 10M

# Запуск в QEMU с диском (для сохранения файловой системы)
qemu-disk: $(KERNEL_BIN) $(DISK_IMG)
	qemu-system-i386 -kernel $(KERNEL_BIN) -hda $(DISK_IMG) -serial stdio -display none

# Очистка
clean:
	rm -f $(BOOT_OBJ) $(KERNEL_OBJ) $(LIB_OBJS) $(MEMORY_OBJS) $(INTERRUPTS_OBJS) $(DRIVERS_OBJS) $(TASK_ASM_OBJS) $(TASK_OBJS) $(FS_OBJS) $(SHELL_OBJS) $(KERNEL_BIN) $(OS_IMAGE) $(DISK_IMG)
	rm -rf isodir
