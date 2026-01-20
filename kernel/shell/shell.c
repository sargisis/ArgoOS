// Shell Implementation

#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "string.h"
#include "heap.h"
#include "timer.h"

#define MAX_COMMANDS 32
static struct shell_command commands[MAX_COMMANDS];
static uint32_t command_count = 0;
static char input_buffer[SHELL_MAX_LINE_LENGTH];
static uint32_t input_pos = 0;

// Command handlers
static int cmd_help(int argc, char** argv);
static int cmd_clear(int argc, char** argv);
static int cmd_echo(int argc, char** argv);
static int cmd_time(int argc, char** argv);
static int cmd_meminfo(int argc, char** argv);
static int cmd_reboot(int argc, char** argv);

void shell_init(void) {
    command_count = 0;
    input_pos = 0;
    memset(input_buffer, 0, SHELL_MAX_LINE_LENGTH);
    memset(commands, 0, sizeof(commands));
    
    // Register built-in commands
    shell_register_command("help", "Show available commands", cmd_help);
    shell_register_command("clear", "Clear the screen", cmd_clear);
    shell_register_command("echo", "Echo arguments", cmd_echo);
    shell_register_command("time", "Show system uptime", cmd_time);
    shell_register_command("meminfo", "Show memory information", cmd_meminfo);
    shell_register_command("reboot", "Reboot the system", cmd_reboot);
}

void shell_register_command(const char* name, const char* description, command_handler_t handler) {
    if (command_count >= MAX_COMMANDS) {
        return; // Too many commands
    }
    
    commands[command_count].name = name;
    commands[command_count].description = description;
    commands[command_count].handler = handler;
    command_count++;
}

struct shell_command* shell_get_command(const char* name) {
    for (uint32_t i = 0; i < command_count; i++) {
        if (strcmp(commands[i].name, name) == 0) {
            return &commands[i];
        }
    }
    return NULL;
}

int shell_process_command(const char* line) {
    if (line == NULL || strlen(line) == 0) {
        return 0;
    }
    
    // Skip leading whitespace
    while (*line == ' ' || *line == '\t') {
        line++;
    }
    
    if (*line == '\0') {
        return 0; // Empty line
    }
    
    // Parse command and arguments
    char* args[SHELL_MAX_ARGS];
    int argc = 0;
    
    // Allocate buffer for parsing
    char* line_copy = (char*)kmalloc(strlen(line) + 1);
    if (line_copy == NULL) {
        return -1;
    }
    strcpy(line_copy, line);
    
    // Tokenize (simple implementation without strtok_r)
    char* start = line_copy;
    char* end = line_copy;
    
    while (argc < SHELL_MAX_ARGS - 1 && *end != '\0') {
        // Skip whitespace
        while (*start == ' ' || *start == '\t' || *start == '\n') {
            start++;
        }
        
        if (*start == '\0') {
            break;
        }
        
        end = start;
        // Find end of token
        while (*end != '\0' && *end != ' ' && *end != '\t' && *end != '\n') {
            end++;
        }
        
        if (*end != '\0') {
            *end = '\0';
            end++;
        }
        
        args[argc++] = start;
        start = end;
    }
    args[argc] = NULL;
    
    if (argc == 0) {
        kfree(line_copy);
        return 0;
    }
    
    // Find and execute command
    struct shell_command* cmd = shell_get_command(args[0]);
    if (cmd == NULL) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_puts("Command not found: ");
        vga_puts(args[0]);
        vga_puts("\n");
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        vga_puts("Type 'help' for available commands.\n");
        kfree(line_copy);
        return -1;
    }
    
    // Execute command
    int result = cmd->handler(argc, args);
    
    kfree(line_copy);
    return result;
}

void shell_run(void) {
    vga_puts("\n");
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts("FlowDay-OS Shell v0.1\n");
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    vga_puts("Type 'help' for available commands.\n");
    vga_puts("\n");
    
    while (1) {
        // Show prompt
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_puts(SHELL_PROMPT);
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        
        // Read command line
        input_pos = 0;
        memset(input_buffer, 0, SHELL_MAX_LINE_LENGTH);
        
        while (1) {
            // Wait for key
            while (!keyboard_is_key_available()) {
                asm volatile("hlt");
            }
            
            char key = keyboard_get_key();
            
            if (key == '\n' || key == '\r') {
                // Execute command
                vga_putchar('\n');
                if (input_pos > 0) {
                    shell_process_command(input_buffer);
                }
                break;
            } else if (key == '\b') {
                // Backspace
                if (input_pos > 0) {
                    input_pos--;
                    input_buffer[input_pos] = '\0';
                    vga_putchar('\b');
                    vga_putchar(' ');
                    vga_putchar('\b');
                }
            } else if (key >= 32 && key < 127) {
                // Printable character
                if (input_pos < SHELL_MAX_LINE_LENGTH - 1) {
                    input_buffer[input_pos++] = key;
                    vga_putchar(key);
                }
            }
        }
    }
}

// Command implementations
static int cmd_help(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    vga_puts("\nAvailable commands:\n");
    vga_puts("==================\n");
    
    for (uint32_t i = 0; i < command_count; i++) {
        vga_puts("  ");
        vga_puts(commands[i].name);
        vga_puts(" - ");
        vga_puts(commands[i].description);
        vga_puts("\n");
    }
    
    vga_puts("\n");
    return 0;
}

static int cmd_clear(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    vga_clear();
    return 0;
}

static int cmd_echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        vga_puts(argv[i]);
        if (i < argc - 1) {
            vga_puts(" ");
        }
    }
    vga_puts("\n");
    return 0;
}

static int cmd_time(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    uint32_t ms = timer_get_ms();
    uint32_t seconds = ms / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    
    vga_puts("System uptime: ");
    vga_putdec(hours);
    vga_puts("h ");
    vga_putdec(minutes % 60);
    vga_puts("m ");
    vga_putdec(seconds % 60);
    vga_puts("s (");
    vga_putdec(ms);
    vga_puts(" ms)\n");
    
    return 0;
}

static int cmd_meminfo(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    extern uint32_t pmm_get_total_pages(void);
    extern uint32_t pmm_get_free_pages(void);
    
    uint32_t total = pmm_get_total_pages();
    uint32_t free = pmm_get_free_pages();
    uint32_t used = total - free;
    
    vga_puts("Memory Information:\n");
    vga_puts("  Total pages: ");
    vga_putdec(total);
    vga_puts(" (");
    vga_putdec(total * 4);
    vga_puts(" KB)\n");
    vga_puts("  Used pages: ");
    vga_putdec(used);
    vga_puts(" (");
    vga_putdec(used * 4);
    vga_puts(" KB)\n");
    vga_puts("  Free pages: ");
    vga_putdec(free);
    vga_puts(" (");
    vga_putdec(free * 4);
    vga_puts(" KB)\n");
    
    return 0;
}

static int cmd_reboot(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    vga_puts("Rebooting system...\n");
    
    // Trigger reboot via keyboard controller
    uint8_t temp;
    do {
        asm volatile("inb %1, %0" : "=a"(temp) : "Nd"(0x64));
    } while (temp & 0x02);
    
    asm volatile("outb %0, %1" :: "a"((uint8_t)0xFE), "Nd"(0x64));
    
    // If that doesn't work, triple fault
    asm volatile("int $0xFF");
    
    return 0;
}
