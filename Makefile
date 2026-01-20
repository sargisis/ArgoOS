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
INCLUDE_DIR = include

# Исходные файлы
BOOT_SRC = $(BOOT_DIR)/multiboot.asm
KERNEL_SRC = $(KERNEL_DIR)/kernel.c
LIB_SRCS = $(LIB_DIR)/vga.c $(LIB_DIR)/string.c

# Объектные файлы
BOOT_OBJ = $(BOOT_DIR)/multiboot.o
KERNEL_OBJ = $(KERNEL_DIR)/kernel.o
LIB_OBJS = $(LIB_DIR)/vga.o $(LIB_DIR)/string.o

# Итоговый образ
KERNEL_BIN = kernel.bin
OS_IMAGE = flowday-os.iso

.PHONY: all clean run qemu iso

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

# Линковка kernel
$(KERNEL_BIN): $(BOOT_OBJ) $(KERNEL_OBJ) $(LIB_OBJS)
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

# Запуск в QEMU (прямая загрузка kernel)
qemu: $(KERNEL_BIN)
	qemu-system-i386 -kernel $(KERNEL_BIN)

# Очистка
clean:
	rm -f $(BOOT_OBJ) $(KERNEL_OBJ) $(LIB_OBJS) $(KERNEL_BIN) $(OS_IMAGE)
	rm -rf isodir
