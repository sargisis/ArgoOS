#include "printf.h"
#include "serial.h"
#include "vga.h"
#include "string.h"

static const char hex_chars[] = "0123456789ABCDEF";

static void itoa(char* buf, int base, uint32_t n) {
    char temp[32];
    int i = 0;

    if (n == 0) {
        temp[i++] = '0';
    } else {
        while (n > 0) {
            temp[i++] = hex_chars[n % base];
            n /= base;
        }
    }

    // Reverse into buf
    for (int j = 0; j < i; j++) {
        buf[j] = temp[i - j - 1];
    }
    buf[i] = '\0';
}

static void itoa_signed(char* buf, int base, int32_t n) {
    if (n < 0 && base == 10) {
        *buf++ = '-';
        itoa(buf, base, (uint32_t)(-n));
    } else {
        itoa(buf, base, (uint32_t)n);
    }
}

int vsprintf(char* buf, const char* format, va_list args) {
    char* p = buf;
    const char* f = format;

    while (*f) {
        if (*f == '%') {
            f++;
            switch (*f) {
                case 's': {
                    char* s = va_arg(args, char*);
                    if (!s) s = "(null)";
                    while (*s) *p++ = *s++;
                    break;
                }
                case 'd': {
                    int32_t n = va_arg(args, int32_t);
                    char tmp[32];
                    itoa_signed(tmp, 10, n);
                    char* t = tmp;
                    while (*t) *p++ = *t++;
                    break;
                }
                case 'u': {
                    uint32_t n = va_arg(args, uint32_t);
                    char tmp[32];
                    itoa(tmp, 10, n);
                    char* t = tmp;
                    while (*t) *p++ = *t++;
                    break;
                }
                case 'x':
                case 'p': {
                    uint32_t n = va_arg(args, uint32_t);
                    char tmp[32];
                    itoa(tmp, 16, n);
                    char* t = tmp;
                    while (*t) *p++ = *t++;
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    *p++ = c;
                    break;
                }
                case '%': {
                    *p++ = '%';
                    break;
                }
                default: {
                    *p++ = *f;
                    break;
                }
            }
        } else {
            *p++ = *f;
        }
        f++;
    }
    *p = '\0';
    return (int)(p - buf);
}

int sprintf(char* buf, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int len = vsprintf(buf, format, args);
    va_end(args);
    return len;
}

void kprintf(const char* format, ...) {
    char buf[1024]; // Safe for kernel debug messages
    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    // Output to serial
    serial_puts(buf);
    
    // Output to VGA (if possible)
    vga_puts(buf);
}
