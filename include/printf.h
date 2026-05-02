#ifndef PRINTF_H
#define PRINTF_H

#include "types.h"

// GCC built-ins for freestanding stdarg
typedef __builtin_va_list va_list;
#define va_start(v,l) __builtin_va_start(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v,l) __builtin_va_arg(v,l)
#define va_copy(d,s) __builtin_va_copy(d,s)

// Standard printf for the kernel
void kprintf(const char* format, ...);

// Formatted string to buffer
int sprintf(char* buf, const char* format, ...);
int vsprintf(char* buf, const char* format, va_list args);

#endif // PRINTF_H
