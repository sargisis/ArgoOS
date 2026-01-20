// PIC (Programmable Interrupt Controller)
// Управление PIC для обработки аппаратных прерываний

#ifndef PIC_H
#define PIC_H

#include "types.h"

// Порты PIC
#define PIC1_COMMAND 0x20
#define PIC1_DATA    0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA    0xA1

// Команды PIC
#define PIC_EOI      0x20  // End Of Interrupt

// ICW1 (Initialization Command Word 1)
#define ICW1_ICW4    0x01  // ICW4 needed
#define ICW1_SINGLE  0x02  // Single mode
#define ICW1_INTERVAL4 0x04  // Call address interval 4
#define ICW1_LEVEL   0x08  // Level triggered mode
#define ICW1_INIT    0x10  // Initialization

// ICW4 (Initialization Command Word 4)
#define ICW4_8086    0x01  // 8086/88 mode
#define ICW4_AUTO    0x02  // Auto EOI
#define ICW4_BUF_SLAVE 0x08  // Buffered mode slave
#define ICW4_BUF_MASTER 0x0C  // Buffered mode master
#define ICW4_SFNM    0x10  // Special fully nested mode

// Инициализация PIC
void pic_init(void);

// Отправка EOI (End Of Interrupt)
void pic_send_eoi(uint8_t irq);

// Маскирование IRQ
void pic_mask_irq(uint8_t irq);

// Размаскирование IRQ
void pic_unmask_irq(uint8_t irq);

#endif // PIC_H
