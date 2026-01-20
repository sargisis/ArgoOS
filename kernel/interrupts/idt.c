// IDT Implementation

#include "idt.h"
#include "pic.h"
#include "vga.h"
#include "string.h"

// IDT таблица
static struct idt_entry idt[IDT_ENTRIES];
static struct idt_ptr idtp;

// Внешние функции из idt.asm
extern void idt_flush(uint32_t);

// Имена исключений для отладки
static const char* exception_messages[] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};

void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = selector;
    idt[num].always0 = 0;
    idt[num].flags = flags | IDT_FLAG_PRESENT;
}

void idt_init(void) {
    // Настраиваем IDT pointer
    idtp.limit = (sizeof(struct idt_entry) * IDT_ENTRIES) - 1;
    idtp.base = (uint32_t)&idt;
    
    // Очищаем IDT
    memset(&idt, 0, sizeof(struct idt_entry) * IDT_ENTRIES);
    
    // Устанавливаем обработчики исключений (0-31)
    idt_set_gate(0, (uint32_t)isr0, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(1, (uint32_t)isr1, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(2, (uint32_t)isr2, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(3, (uint32_t)isr3, 0x08, IDT_GATE_32BIT_TRAP);
    idt_set_gate(4, (uint32_t)isr4, 0x08, IDT_GATE_32BIT_TRAP);
    idt_set_gate(5, (uint32_t)isr5, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(6, (uint32_t)isr6, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(7, (uint32_t)isr7, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(8, (uint32_t)isr8, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(9, (uint32_t)isr9, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(10, (uint32_t)isr10, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(11, (uint32_t)isr11, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(12, (uint32_t)isr12, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(13, (uint32_t)isr13, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(14, (uint32_t)isr14, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(15, (uint32_t)isr15, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(16, (uint32_t)isr16, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(17, (uint32_t)isr17, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(18, (uint32_t)isr18, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(19, (uint32_t)isr19, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(20, (uint32_t)isr20, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(21, (uint32_t)isr21, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(22, (uint32_t)isr22, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(23, (uint32_t)isr23, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(24, (uint32_t)isr24, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(25, (uint32_t)isr25, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(26, (uint32_t)isr26, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(27, (uint32_t)isr27, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(28, (uint32_t)isr28, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(29, (uint32_t)isr29, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(30, (uint32_t)isr30, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(31, (uint32_t)isr31, 0x08, IDT_GATE_32BIT_INT);
    
    // Устанавливаем IRQ обработчики (32-47)
    idt_set_gate(32, (uint32_t)irq0, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(33, (uint32_t)irq1, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(34, (uint32_t)irq2, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(35, (uint32_t)irq3, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(36, (uint32_t)irq4, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(37, (uint32_t)irq5, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(38, (uint32_t)irq6, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(39, (uint32_t)irq7, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(40, (uint32_t)irq8, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(41, (uint32_t)irq9, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(42, (uint32_t)irq10, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(43, (uint32_t)irq11, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(44, (uint32_t)irq12, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(45, (uint32_t)irq13, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(46, (uint32_t)irq14, 0x08, IDT_GATE_32BIT_INT);
    idt_set_gate(47, (uint32_t)irq15, 0x08, IDT_GATE_32BIT_INT);
    
    // Загружаем IDT
    idt_flush((uint32_t)&idtp);
}

// Обработчик исключений
void isr_handler(uint32_t int_no, uint32_t err_code) {
    vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
    vga_puts("\n!!! EXCEPTION !!!\n");
    vga_puts("Exception #");
    vga_putdec(int_no);
    vga_puts(": ");
    
    if (int_no < 32) {
        vga_puts(exception_messages[int_no]);
    } else {
        vga_puts("Unknown Exception");
    }
    
    if (err_code != 0) {
        vga_puts("\nError Code: 0x");
        vga_puthex(err_code);
    }
    
    vga_puts("\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    
    // Для некоторых исключений можно попытаться продолжить работу
    // Для других - лучше остановить систему
    if (int_no == 14) { // Page Fault
        uint32_t fault_addr;
        asm volatile("mov %%cr2, %0" : "=r"(fault_addr));
        vga_puts("Page Fault Address: 0x");
        vga_puthex(fault_addr);
        vga_puts("\n");
    }
    
    // В реальной ОС здесь можно попытаться обработать исключение
    // Пока просто останавливаемся
    // asm volatile("cli; hlt");
}

// Внешние обработчики устройств
extern void timer_handler(void);
extern void keyboard_handler(void);

// Обработчик IRQ
void irq_handler(uint32_t int_no) {
    // Преобразуем номер прерывания в номер IRQ (32-47 -> 0-15)
    uint8_t irq = int_no - 32;
    
    // Вызываем конкретные обработчики устройств
    switch (irq) {
        case 0:  // Timer
            timer_handler();
            break;
        case 1:  // Keyboard
            keyboard_handler();
            break;
        default:
            // Unknown IRQ
            break;
    }
    
    // Отправляем EOI (End Of Interrupt) в PIC
    pic_send_eoi(irq);
}
