# FlowDay-OS

Минималистичная операционная система, написанная с нуля на C и ассемблере x86.

## Особенности

- ✅ Multiboot 1 совместимость
- ✅ Freestanding режим (без стандартной библиотеки
- ✅ Собственная реализация базовых функций (string, VGA)
- ✅ Минимализм и скорость
- ✅ Глубокое понимание железа

## Требования

- `nasm` - ассемблер
- `gcc` - компилятор C (с поддержкой 32-bit)
- `ld` - линкер
- `grub-mkrescue` - для создания ISO образа
- `qemu-system-i386` - для эмуляции

### Установка зависимостей (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install nasm gcc-multilib binutils grub-pc-bin qemu-system-x86
```

## Сборка

```bash
make
```

Это создаст `kernel.bin` - скомпилированное ядро.

## Запуск

### Вариант 1: Прямая загрузка через QEMU

```bash
make qemu
```

### Вариант 2: ISO образ (рекомендуется)

```bash
make iso
make run
```

## Структура проекта

```
FlowDay-OS/
├── boot/              # Multiboot entry point
│   └── multiboot.asm
├── kernel/            # Основное ядро
│   ├── kernel.c       # Главный файл ядра
│   └── lib/           # Библиотечные функции
│       ├── vga.c      # VGA драйвер
│       └── string.c   # Функции работы со строками
├── include/           # Заголовочные файлы
│   ├── kernel.h
│   ├── multiboot.h
│   ├── vga.h
│   ├── string.h
│   └── types.h
├── kernel.ld          # Linker script
├── grub.cfg           # GRUB конфигурация
└── Makefile           # Сборка проекта
```

## Текущий статус

- [x] Multiboot загрузка
- [x] Базовое ядро
- [x] VGA текстовый режим
- [x] Собственные функции работы со строками
- [ ] Управление памятью
- [ ] Прерывания (IDT)
- [ ] Драйверы устройств (клавиатура, таймер)
- [ ] Многозадачность
- [ ] Файловая система

## Лицензия

MIT License

## Автор

FlowDay-OS Development Team
