// Timer Driver Implementation (PIT)

#include "timer.h"
#include "idt.h"
#include "pic.h"
#include "vga.h"

static uint32_t timer_ticks = 0;
static uint32_t timer_frequency = TIMER_FREQUENCY;
static timer_callback_t timer_callback = NULL;

// External task scheduler
extern void task_switch(void);
extern struct task* task_get_current(void);

// Timer interrupt handler (IRQ 0)
void timer_handler(void) {
    timer_ticks++;
    
    // Call registered callback if exists
    if (timer_callback != NULL) {
        timer_callback();
    }
    
    // Trigger task switch only if we have multiple tasks
    // For now, disable automatic task switching to allow shell to work
    // TODO: Re-enable when we have proper task management
    // struct task* current = task_get_current();
    // if (current != NULL) {
    //     current->time_slice--;
    //     if (current->time_slice == 0) {
    //         task_switch();
    //     }
    // }
}

void timer_init(uint32_t frequency) {
    timer_frequency = frequency;
    timer_ticks = 0;
    
    // Calculate divisor for desired frequency
    uint32_t divisor = PIT_FREQUENCY / frequency;
    
    // Send command byte to PIT
    // Channel 0, Access mode: lobyte/hibyte, Mode 3 (square wave), Binary mode
    uint8_t command = 0x36; // 00110110
    
    asm volatile("outb %0, %1" :: "a"((uint8_t)command), "Nd"(PIT_COMMAND));
    
    // Send divisor (low byte then high byte)
    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);
    
    asm volatile("outb %0, %1" :: "a"(low), "Nd"(PIT_CHANNEL0_DATA));
    asm volatile("outb %0, %1" :: "a"(high), "Nd"(PIT_CHANNEL0_DATA));
    
    // Unmask IRQ 0 (timer)
    pic_unmask_irq(0);
}

uint32_t timer_get_ticks(void) {
    return timer_ticks;
}

uint32_t timer_get_ms(void) {
    return (timer_ticks * 1000) / timer_frequency;
}

void timer_sleep(uint32_t milliseconds) {
    uint32_t start_ticks = timer_ticks;
    uint32_t target_ticks = start_ticks + (milliseconds * timer_frequency / 1000);
    
    while (timer_ticks < target_ticks) {
        asm volatile("hlt"); // Wait for interrupt
    }
}

void timer_register_callback(timer_callback_t callback) {
    timer_callback = callback;
}
