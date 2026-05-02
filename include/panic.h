#ifndef PANIC_H
#define PANIC_H

#include "types.h"

#define PANIC(msg) kpanic(msg, __FILE__, __LINE__)
#define ASSERT(cond) if (!(cond)) { kpanic("Assertion failed: " #cond, __FILE__, __LINE__); }

void kpanic(const char* message, const char* file, uint32_t line);

#endif // PANIC_H
