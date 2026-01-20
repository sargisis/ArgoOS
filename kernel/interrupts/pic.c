// PIC Implementation

#include "pic.h"

void pic_init(void) {
    // Сохраняем текущие маски
    uint8_t a1, a2;
    
    asm volatile("inb %1, %0" : "=a"(a1) : "Nd"(PIC1_DATA));
    asm volatile("inb %1, %0" : "=a"(a2) : "Nd"(PIC2_DATA));
    
    // Инициализация PIC1
    uint8_t val;
    val = ICW1_INIT | ICW1_ICW4;
    asm volatile("outb %0, %1" :: "a"(val), "Nd"(PIC1_COMMAND));
    val = 0x20;
    asm volatile("outb %0, %1" :: "a"(val), "Nd"(PIC1_DATA));  // IRQ 0-7 -> INT 0x20-0x27
    val = 0x04;
    asm volatile("outb %0, %1" :: "a"(val), "Nd"(PIC1_DATA));  // Slave на IRQ2
    val = ICW4_8086;
    asm volatile("outb %0, %1" :: "a"(val), "Nd"(PIC1_DATA));
    
    // Инициализация PIC2
    val = ICW1_INIT | ICW1_ICW4;
    asm volatile("outb %0, %1" :: "a"(val), "Nd"(PIC2_COMMAND));
    val = 0x28;
    asm volatile("outb %0, %1" :: "a"(val), "Nd"(PIC2_DATA));  // IRQ 8-15 -> INT 0x28-0x2F
    val = 0x02;
    asm volatile("outb %0, %1" :: "a"(val), "Nd"(PIC2_DATA));  // Slave ID = 2
    val = ICW4_8086;
    asm volatile("outb %0, %1" :: "a"(val), "Nd"(PIC2_DATA));
    
    // Восстанавливаем маски
    asm volatile("outb %0, %1" :: "a"(a1), "Nd"(PIC1_DATA));
    asm volatile("outb %0, %1" :: "a"(a2), "Nd"(PIC2_DATA));
}

void pic_send_eoi(uint8_t irq) {
    uint8_t eoi = PIC_EOI;
    if (irq >= 8) {
        // Отправляем EOI в slave PIC
        asm volatile("outb %0, %1" :: "a"(eoi), "Nd"(PIC2_COMMAND));
    }
    // Отправляем EOI в master PIC
    asm volatile("outb %0, %1" :: "a"(eoi), "Nd"(PIC1_COMMAND));
}

void pic_mask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    value |= (1 << irq);
    asm volatile("outb %0, %1" :: "a"(value), "Nd"(port));
}

void pic_unmask_irq(uint8_t irq) {
    uint16_t port;
    uint8_t value;
    
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    
    asm volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    value &= ~(1 << irq);
    asm volatile("outb %0, %1" :: "a"(value), "Nd"(port));
}
