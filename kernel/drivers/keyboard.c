// Keyboard Driver Implementation (PS/2)

#include "keyboard.h"
#include "idt.h"
#include "pic.h"
#include "vga.h"

static char last_key = 0;
static int key_available = 0;
static keyboard_callback_t keyboard_callback = NULL;

// US QWERTY keyboard scancode to ASCII mapping (set 1)
static const char keyboard_map[128] = {
    0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,   'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0,   '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   '-', 0,   0,   0,   '+', 0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

// US QWERTY keyboard scancode to ASCII mapping (shifted)
static const char keyboard_map_shift[128] = {
    0,   27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,   'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
    0,   '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0,   ' ', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   '-', 0,   0,   0,   '+', 0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

static int shift_pressed = 0;
static int caps_lock = 0;

// Keyboard interrupt handler (IRQ 1)
void keyboard_handler(void) {
    uint8_t scancode;
    
    // Read scancode from keyboard data port
    asm volatile("inb %1, %0" : "=a"(scancode) : "Nd"(KEYBOARD_DATA_PORT));
    
    // Handle special keys
    if (scancode == 0x2A || scancode == 0x36) {
        // Left or right shift pressed
        shift_pressed = 1;
        return;
    }
    
    if (scancode == 0xAA || scancode == 0xB6) {
        // Left or right shift released
        shift_pressed = 0;
        return;
    }
    
    if (scancode == 0x3A) {
        // Caps Lock pressed
        caps_lock = !caps_lock;
        return;
    }
    
    // Handle key release (scancode > 0x80)
    if (scancode & 0x80) {
        return; // Key released, ignore
    }
    
    // Convert scancode to ASCII
    char character = keyboard_scancode_to_ascii(scancode);
    
    if (character != 0) {
        last_key = character;
        key_available = 1;
        
        // Call registered callback if exists
        if (keyboard_callback != NULL) {
            keyboard_callback(scancode, character);
        }
    }
}

void keyboard_init(void) {
    last_key = 0;
    key_available = 0;
    shift_pressed = 0;
    caps_lock = 0;
    
    // Unmask IRQ 1 (keyboard)
    pic_unmask_irq(1);
}

// Polling функция для чтения клавиатуры (без прерываний)
char keyboard_poll(void) {
    uint8_t status;
    
    // Проверяем, есть ли данные в буфере клавиатуры
    asm volatile("inb %1, %0" : "=a"(status) : "Nd"(KEYBOARD_STATUS_PORT));
    
    // Если есть данные (бит 0 установлен)
    if (status & KEYBOARD_STATUS_OUTPUT_FULL) {
        uint8_t scancode;
        asm volatile("inb %1, %0" : "=a"(scancode) : "Nd"(KEYBOARD_DATA_PORT));
        
        // Обрабатываем scancode (та же логика, что и в keyboard_handler)
        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = 1;
            return 0;
        }
        
        if (scancode == 0xAA || scancode == 0xB6) {
            shift_pressed = 0;
            return 0;
        }
        
        if (scancode == 0x3A) {
            caps_lock = !caps_lock;
            return 0;
        }
        
        // Игнорируем отпускание клавиш
        if (scancode & 0x80) {
            return 0;
        }
        
        // Конвертируем в ASCII
        char character = keyboard_scancode_to_ascii(scancode);
        return character;
    }
    
    return 0; // Нет данных
}

char keyboard_get_key(void) {
    if (key_available) {
        key_available = 0;
        return last_key;
    }
    return 0;
}

int keyboard_is_key_available(void) {
    // Используем polling вместо проверки флага
    char key = keyboard_poll();
    if (key != 0) {
        last_key = key;
        key_available = 1;
        return 1;
    }
    return 0;
}

void keyboard_register_callback(keyboard_callback_t callback) {
    keyboard_callback = callback;
}

char keyboard_scancode_to_ascii(uint8_t scancode) {
    if (scancode >= 128) {
        return 0; // Invalid scancode
    }
    
    int use_shift = shift_pressed ^ caps_lock;
    const char* map = use_shift ? keyboard_map_shift : keyboard_map;
    
    return map[scancode];
}
