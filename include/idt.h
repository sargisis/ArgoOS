// IDT (Interrupt Descriptor Table)
// Управление прерываниями и исключениями

#ifndef IDT_H
#define IDT_H

#include "types.h"

// Количество прерываний
#define IDT_ENTRIES 256

// Типы шлюзов (gate types)
#define IDT_GATE_TASK      0x5
#define IDT_GATE_16BIT_INT 0x6
#define IDT_GATE_16BIT_TRAP 0x7
#define IDT_GATE_32BIT_INT 0xE
#define IDT_GATE_32BIT_TRAP 0xF

// Флаги
#define IDT_FLAG_PRESENT   0x80
#define IDT_FLAG_RING0     0x00
#define IDT_FLAG_RING3     0x60

// Структура IDT entry
struct idt_entry {
    uint16_t base_low;      // Младшие 16 бит адреса обработчика
    uint16_t selector;      // Селектор сегмента кода
    uint8_t  always0;       // Всегда 0
    uint8_t  flags;         // Флаги (тип, привилегии, присутствие)
    uint16_t base_high;     // Старшие 16 бит адреса обработчика
} __attribute__((packed));

// Структура IDT pointer для LIDT
struct idt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// Инициализация IDT
void idt_init(void);

// Установить обработчик прерывания
void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags);

// Обработчики прерываний (определены в idt.asm)
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

// IRQ обработчики
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

// C обработчики (вызываются из ассемблера)
void isr_handler(uint32_t int_no, uint32_t err_code);
void irq_handler(uint32_t int_no);

#endif // IDT_H
