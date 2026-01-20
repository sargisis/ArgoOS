// Timer Driver (PIT - Programmable Interval Timer)
// System timer and time management

#ifndef TIMER_H
#define TIMER_H

#include "types.h"

// PIT ports
#define PIT_CHANNEL0_DATA 0x40
#define PIT_CHANNEL1_DATA 0x41
#define PIT_CHANNEL2_DATA 0x42
#define PIT_COMMAND       0x43

// PIT frequency (Hz)
#define PIT_FREQUENCY 1193180

// Default timer frequency (Hz) - 100Hz = 10ms per tick
#define TIMER_FREQUENCY 100

// Timer callback function type
typedef void (*timer_callback_t)(void);

// Initialize timer
void timer_init(uint32_t frequency);

// Get current tick count
uint32_t timer_get_ticks(void);

// Get current time in milliseconds
uint32_t timer_get_ms(void);

// Sleep for specified milliseconds
void timer_sleep(uint32_t milliseconds);

// Register timer callback
void timer_register_callback(timer_callback_t callback);

#endif // TIMER_H
