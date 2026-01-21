// Serial Port Driver (COM1)
// Simple UART driver for debugging and I/O

#ifndef SERIAL_H
#define SERIAL_H

#include "types.h"

// COM1 port addresses
#define SERIAL_COM1_BASE 0x3F8

// Serial port registers
#define SERIAL_DATA_PORT(base)      (base)
#define SERIAL_FIFO_COMMAND_PORT(base)   (base + 2)
#define SERIAL_LINE_COMMAND_PORT(base)   (base + 3)
#define SERIAL_MODEM_COMMAND_PORT(base)  (base + 4)
#define SERIAL_LINE_STATUS_PORT(base)    (base + 5)

// Line status bits
#define SERIAL_LINE_STATUS_DATA_READY 0x01
#define SERIAL_LINE_STATUS_TRANSMIT_EMPTY 0x20

// Initialize serial port
void serial_init(void);

// Write a character to serial port
void serial_putchar(char c);

// Read a character from serial port (returns 0 if no data)
char serial_getchar(void);

// Check if data is available
int serial_is_data_available(void);

// Write a string to serial port
void serial_puts(const char* str);

// Output decimal number
void serial_putdec(uint32_t value);

// Output hexadecimal number
void serial_puthex(uint32_t value);

#endif // SERIAL_H
