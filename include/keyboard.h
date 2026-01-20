// Keyboard Driver (PS/2)
// Keyboard input handling

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

// Keyboard ports
#define KEYBOARD_DATA_PORT    0x60
#define KEYBOARD_STATUS_PORT  0x64
#define KEYBOARD_COMMAND_PORT 0x64

// Keyboard status bits
#define KEYBOARD_STATUS_OUTPUT_FULL 0x01
#define KEYBOARD_STATUS_INPUT_FULL  0x02

// Keyboard callback function type
typedef void (*keyboard_callback_t)(uint8_t scancode, char character);

// Initialize keyboard
void keyboard_init(void);

// Get last pressed key
char keyboard_get_key(void);

// Check if key is available
int keyboard_is_key_available(void);

// Register keyboard callback
void keyboard_register_callback(keyboard_callback_t callback);

// Convert scancode to ASCII character
char keyboard_scancode_to_ascii(uint8_t scancode);

#endif // KEYBOARD_H
