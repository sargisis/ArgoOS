// Shell Implementation

#include "shell.h"
#include "vga.h"
#include "serial.h"
#include "keyboard.h"
#include "string.h"
#include "heap.h"
#include "timer.h"
#include "fs.h"

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
static int cmd_ls(int argc, char** argv);
static int cmd_cat(int argc, char** argv);
static int cmd_mkdir(int argc, char** argv);
static int cmd_rm(int argc, char** argv);
static int cmd_pwd(int argc, char** argv);
static int cmd_cd(int argc, char** argv);
static int cmd_touch(int argc, char** argv);

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
    shell_register_command("ls", "List directory contents", cmd_ls);
    shell_register_command("cat", "Display file contents", cmd_cat);
    shell_register_command("mkdir", "Create directory", cmd_mkdir);
    shell_register_command("rm", "Remove file or directory", cmd_rm);
    shell_register_command("pwd", "Print working directory", cmd_pwd);
    shell_register_command("cd", "Change directory", cmd_cd);
    shell_register_command("touch", "Create empty file", cmd_touch);
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
        serial_puts("Command not found: ");
        serial_puts(args[0]);
        serial_puts("\n");
        serial_puts("Type 'help' for available commands.\n");
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
    
    // Test output to verify shell is running
    vga_puts("Shell is ready. Prompt should appear below.\n");
    
    while (1) {
        // Show prompt
        vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
        vga_puts(SHELL_PROMPT);
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
        
        // Read command line
        input_pos = 0;
        memset(input_buffer, 0, SHELL_MAX_LINE_LENGTH);
        
        // Main input loop - simplified
        while (1) {
            // Ensure interrupts are enabled
            asm volatile("sti");
            
            // Simple busy-wait for keyboard (for testing)
            // In production, we'd use interrupts properly
            if (keyboard_is_key_available()) {
                char key = keyboard_get_key();
                
                if (key == '\n' || key == '\r') {
                    // Execute command
                    vga_putchar('\n');
                    if (input_pos > 0) {
                        shell_process_command(input_buffer);
                    }
                    break; // Exit inner loop, show prompt again
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
            
            // Wait for interrupt (this will wake up on keyboard IRQ)
            asm volatile("hlt");
        }
    }
}

// Command implementations
static int cmd_help(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    serial_puts("\nAvailable commands:\n");
    serial_puts("==================\n");
    
    for (uint32_t i = 0; i < command_count; i++) {
        serial_puts("  ");
        serial_puts(commands[i].name);
        serial_puts(" - ");
        serial_puts(commands[i].description);
        serial_puts("\n");
    }
    
    serial_puts("\n");
    return 0;
}

static int cmd_clear(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    vga_clear();
    return 0;
}

static int cmd_echo(int argc, char** argv) {
    if (argc < 2) {
        serial_puts("\n");
        return 0;
    }
    
    // Check for redirection (echo "text" > file)
    int redirect_index = -1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], ">") == 0 && i + 1 < argc) {
            redirect_index = i;
            break;
        }
    }
    
    if (redirect_index > 0) {
        // Write to file
        const char* filename = argv[redirect_index + 1];
        int fd = fs_open(filename, FS_MODE_WRITE);
        if (fd < 0) {
            serial_puts("Error: Cannot create file '");
            serial_puts(filename);
            serial_puts("'\n");
            return -1;
        }
        
        // Write all arguments before ">"
        for (int i = 1; i < redirect_index; i++) {
            fs_write(fd, argv[i], strlen(argv[i]));
            if (i < redirect_index - 1) {
                fs_write(fd, " ", 1);
            }
        }
        fs_write(fd, "\n", 1);
        fs_close(fd);
        return 0;
    }
    
    // Normal echo - just print
    for (int i = 1; i < argc; i++) {
        serial_puts(argv[i]);
        if (i < argc - 1) {
            serial_puts(" ");
        }
    }
    serial_puts("\n");
    return 0;
}

static int cmd_time(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    uint32_t ms = timer_get_ms();
    uint32_t seconds = ms / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    
    serial_puts("System uptime: ");
    serial_putdec(hours);
    serial_puts("h ");
    serial_putdec(minutes % 60);
    serial_puts("m ");
    serial_putdec(seconds % 60);
    serial_puts("s (");
    serial_putdec(ms);
    serial_puts(" ms)\n");
    
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
    
    serial_puts("Memory Information:\n");
    serial_puts("  Total pages: ");
    serial_putdec(total);
    serial_puts(" (");
    serial_putdec(total * 4);
    serial_puts(" KB)\n");
    serial_puts("  Used pages: ");
    serial_putdec(used);
    serial_puts(" (");
    serial_putdec(used * 4);
    serial_puts(" KB)\n");
    serial_puts("  Free pages: ");
    serial_putdec(free);
    serial_puts(" (");
    serial_putdec(free * 4);
    serial_puts(" KB)\n");
    
    return 0;
}

static int cmd_reboot(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    serial_puts("Rebooting system...\n");
    
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

static int cmd_ls(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : ".";
    
    struct dirent entries[64];
    int count = fs_readdir(path, entries, 64);
    
    if (count < 0) {
        serial_puts("Error: Cannot read directory\n");
        return -1;
    }
    
    if (count == 0) {
        serial_puts("(empty)\n");
        return 0;
    }
    
    for (int i = 0; i < count; i++) {
        if (entries[i].type == FS_TYPE_DIR) {
            serial_puts("[");
            serial_puts(entries[i].name);
            serial_puts("]");
        } else {
            serial_puts(entries[i].name);
            serial_puts(" (");
            serial_putdec(entries[i].size);
            serial_puts(" bytes)");
        }
        serial_puts("\n");
    }
    
    return 0;
}

static int cmd_cat(int argc, char** argv) {
    if (argc < 2) {
        serial_puts("Usage: cat <file>\n");
        return -1;
    }
    
    int fd = fs_open(argv[1], FS_MODE_READ);
    if (fd < 0) {
        serial_puts("Error: Cannot open file '");
        serial_puts(argv[1]);
        serial_puts("'\n");
        return -1;
    }
    
    char buffer[256];
    int32_t bytes_read;
    
    while ((bytes_read = fs_read(fd, buffer, 255)) > 0) {
        buffer[bytes_read] = '\0';
        serial_puts(buffer);
    }
    
    serial_puts("\n");
    fs_close(fd);
    return 0;
}

static int cmd_mkdir(int argc, char** argv) {
    if (argc < 2) {
        serial_puts("Usage: mkdir <directory>\n");
        return -1;
    }
    
    if (fs_mkdir(argv[1]) < 0) {
        serial_puts("Error: Cannot create directory '");
        serial_puts(argv[1]);
        serial_puts("'\n");
        return -1;
    }
    
    return 0;
}

static int cmd_rm(int argc, char** argv) {
    if (argc < 2) {
        serial_puts("Usage: rm <file|directory>\n");
        return -1;
    }
    
    if (fs_remove(argv[1]) < 0) {
        serial_puts("Error: Cannot remove '");
        serial_puts(argv[1]);
        serial_puts("'\n");
        return -1;
    }
    
    return 0;
}

static int cmd_pwd(int argc, char** argv) {
    (void)argc;
    (void)argv;
    
    char path[256];
    fs_get_current_path(path, 256);
    serial_puts(path);
    serial_puts("\n");
    return 0;
}

static int cmd_cd(int argc, char** argv) {
    const char* path = (argc > 1) ? argv[1] : "/";
    
    // Resolve path
    void* dir = fs_resolve_path_export(path);
    if (!dir) {
        serial_puts("Error: Directory not found: ");
        serial_puts(path);
        serial_puts("\n");
        return -1;
    }
    
    if (fs_type(path) != FS_TYPE_DIR) {
        serial_puts("Error: Not a directory: ");
        serial_puts(path);
        serial_puts("\n");
        return -1;
    }
    
    if (fs_set_current_dir(dir) < 0) {
        serial_puts("Error: Cannot change directory\n");
        return -1;
    }
    
    return 0;
}

static int cmd_touch(int argc, char** argv) {
    if (argc < 2) {
        serial_puts("Usage: touch <file>\n");
        return -1;
    }
    
    // Create or update file timestamp (for now, just create empty file)
    int fd = fs_open(argv[1], FS_MODE_WRITE);
    if (fd < 0) {
        serial_puts("Error: Cannot create file '");
        serial_puts(argv[1]);
        serial_puts("'\n");
        return -1;
    }
    
    fs_close(fd);
    return 0;
}
