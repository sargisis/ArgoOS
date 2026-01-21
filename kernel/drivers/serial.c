// Serial Port Driver Implementation (COM1)

#include "serial.h"

static uint16_t serial_port = SERIAL_COM1_BASE;

// Helper functions for port I/O (inline assembly)
static inline void outb(uint16_t port, uint8_t value) {
    asm volatile("outb %0, %1" : : "a"((uint8_t)value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t result;
    asm volatile("inb %1, %0" : "=a"(result) : "Nd"(port));
    return result;
}

// Initialize COM1 serial port
void serial_init(void) {
    // Disable interrupts
    outb(SERIAL_MODEM_COMMAND_PORT(serial_port), 0x00);
    
    // Enable DLAB (set baud rate divisor)
    outb(SERIAL_LINE_COMMAND_PORT(serial_port), 0x80);
    
    // Set divisor to 3 (38400 baud)
    // Divisor = 115200 / desired_baud
    // 115200 / 38400 = 3
    outb(SERIAL_DATA_PORT(serial_port), 0x03);
    outb(SERIAL_DATA_PORT(serial_port) + 1, 0x00);
    
    // 8 bits, no parity, one stop bit
    outb(SERIAL_LINE_COMMAND_PORT(serial_port), 0x03);
    
    // Enable FIFO, clear them, with 14-byte threshold
    outb(SERIAL_FIFO_COMMAND_PORT(serial_port), 0xC7);
    
    // IRQs enabled, RTS/DSR set
    outb(SERIAL_MODEM_COMMAND_PORT(serial_port), 0x0B);
}

// Wait for serial port to be ready to transmit
static void serial_wait_transmit(void) {
    while ((inb(SERIAL_LINE_STATUS_PORT(serial_port)) & SERIAL_LINE_STATUS_TRANSMIT_EMPTY) == 0) {
        // Wait
    }
}

// Write a character to serial port
void serial_putchar(char c) {
    serial_wait_transmit();
    outb(SERIAL_DATA_PORT(serial_port), c);
}

// Read a character from serial port (returns 0 if no data)
char serial_getchar(void) {
    if (serial_is_data_available()) {
        return inb(SERIAL_DATA_PORT(serial_port));
    }
    return 0;
}

// Check if data is available
int serial_is_data_available(void) {
    return (inb(SERIAL_LINE_STATUS_PORT(serial_port)) & SERIAL_LINE_STATUS_DATA_READY) != 0;
}

// Write a string to serial port
void serial_puts(const char* str) {
    while (*str) {
        if (*str == '\n') {
            serial_putchar('\r'); // Add carriage return before newline
        }
        serial_putchar(*str++);
    }
}

// Helper function to convert number to string and output
static void serial_putdec_internal(uint32_t value) {
    if (value == 0) {
        serial_putchar('0');
        return;
    }
    
    char buffer[12];
    int pos = 0;
    
    while (value > 0) {
        buffer[pos++] = '0' + (value % 10);
        value /= 10;
    }
    
    // Output in reverse order
    while (pos > 0) {
        serial_putchar(buffer[--pos]);
    }
}

// Output decimal number
void serial_putdec(uint32_t value) {
    serial_putdec_internal(value);
}

// Output hexadecimal number
void serial_puthex(uint32_t value) {
    serial_puts("0x");
    if (value == 0) {
        serial_putchar('0');
        return;
    }
    
    char buffer[9];
    int pos = 0;
    
    while (value > 0) {
        uint8_t digit = value & 0xF;
        buffer[pos++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        value >>= 4;
    }
    
    // Output in reverse order
    while (pos > 0) {
        serial_putchar(buffer[--pos]);
    }
}
