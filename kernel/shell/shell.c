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
static int cmd_mv(int argc, char** argv);
static int cmd_cp(int argc, char** argv);
static int cmd_find(int argc, char** argv);
static int cmd_grep(int argc, char** argv);
static int cmd_wc(int argc, char** argv);
static int cmd_head(int argc, char** argv);
static int cmd_tail(int argc, char** argv);

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
    shell_register_command("mv", "Move or rename file", cmd_mv);
    shell_register_command("cp", "Copy file", cmd_cp);
    shell_register_command("find", "Find files", cmd_find);
    shell_register_command("grep", "Search text in files", cmd_grep);
    shell_register_command("wc", "Word and line count", cmd_wc);
    shell_register_command("head", "Show first lines of file", cmd_head);
    shell_register_command("tail", "Show last lines of file", cmd_tail);
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

static int cmd_mv(int argc, char** argv) {
    if (argc < 3) {
        serial_puts("Usage: mv <source> <destination>\n");
        return -1;
    }
    
    const char* src = argv[1];
    const char* dst = argv[2];
    
    // Check if source exists
    if (!fs_exists(src)) {
        serial_puts("Error: Source file not found: ");
        serial_puts(src);
        serial_puts("\n");
        return -1;
    }
    
    // Check if source is a directory (not supported for now)
    if (fs_type(src) == FS_TYPE_DIR) {
        serial_puts("Error: Moving directories not yet supported\n");
        return -1;
    }
    
    // Open source file
    int src_fd = fs_open(src, FS_MODE_READ);
    if (src_fd < 0) {
        serial_puts("Error: Cannot open source file\n");
        return -1;
    }
    
    // Get source file size
    uint32_t file_size = fs_size(src_fd);
    
    // Read source file
    char* buffer = (char*)kmalloc(file_size);
    if (!buffer) {
        serial_puts("Error: Out of memory\n");
        fs_close(src_fd);
        return -1;
    }
    
    int32_t bytes_read = fs_read(src_fd, buffer, file_size);
    fs_close(src_fd);
    
    if (bytes_read < 0 || (uint32_t)bytes_read != file_size) {
        serial_puts("Error: Cannot read source file\n");
        kfree(buffer);
        return -1;
    }
    
    // Create destination file
    int dst_fd = fs_open(dst, FS_MODE_WRITE);
    if (dst_fd < 0) {
        serial_puts("Error: Cannot create destination file\n");
        kfree(buffer);
        return -1;
    }
    
    // Write to destination
    int32_t bytes_written = fs_write(dst_fd, buffer, file_size);
    fs_close(dst_fd);
    kfree(buffer);
    
    if (bytes_written < 0 || (uint32_t)bytes_written != file_size) {
        serial_puts("Error: Cannot write to destination file\n");
        return -1;
    }
    
    // Remove source file
    if (fs_remove(src) < 0) {
        serial_puts("Warning: File copied but source could not be removed\n");
        return -1;
    }
    
    return 0;
}

static int cmd_cp(int argc, char** argv) {
    if (argc < 3) {
        serial_puts("Usage: cp <source> <destination>\n");
        return -1;
    }
    
    const char* src = argv[1];
    const char* dst = argv[2];
    
    // Check if source exists
    if (!fs_exists(src)) {
        serial_puts("Error: Source file not found: ");
        serial_puts(src);
        serial_puts("\n");
        return -1;
    }
    
    // Check if source is a directory (not supported for now)
    if (fs_type(src) == FS_TYPE_DIR) {
        serial_puts("Error: Copying directories not yet supported\n");
        return -1;
    }
    
    // Open source file
    int src_fd = fs_open(src, FS_MODE_READ);
    if (src_fd < 0) {
        serial_puts("Error: Cannot open source file\n");
        return -1;
    }
    
    // Get source file size
    uint32_t file_size = fs_size(src_fd);
    
    // Read source file
    char* buffer = (char*)kmalloc(file_size);
    if (!buffer) {
        serial_puts("Error: Out of memory\n");
        fs_close(src_fd);
        return -1;
    }
    
    int32_t bytes_read = fs_read(src_fd, buffer, file_size);
    fs_close(src_fd);
    
    if (bytes_read < 0 || (uint32_t)bytes_read != file_size) {
        serial_puts("Error: Cannot read source file\n");
        kfree(buffer);
        return -1;
    }
    
    // Create destination file
    int dst_fd = fs_open(dst, FS_MODE_WRITE);
    if (dst_fd < 0) {
        serial_puts("Error: Cannot create destination file\n");
        kfree(buffer);
        return -1;
    }
    
    // Write to destination
    int32_t bytes_written = fs_write(dst_fd, buffer, file_size);
    fs_close(dst_fd);
    kfree(buffer);
    
    if (bytes_written < 0 || (uint32_t)bytes_written != file_size) {
        serial_puts("Error: Cannot write to destination file\n");
        return -1;
    }
    
    return 0;
}

static int cmd_find(int argc, char** argv) {
    const char* search_path = (argc > 1) ? argv[1] : ".";
    const char* pattern = (argc > 2) ? argv[2] : NULL;
    
    if (!pattern) {
        serial_puts("Usage: find <path> <pattern>\n");
        serial_puts("Example: find / file.txt\n");
        return -1;
    }
    
    // Simple search in specified directory
    struct dirent entries[64];
    int count = fs_readdir(search_path, entries, 64);
    
    if (count < 0) {
        serial_puts("Error: Cannot read directory: ");
        serial_puts(search_path);
        serial_puts("\n");
        return -1;
    }
    
    int found = 0;
    for (int i = 0; i < count; i++) {
        // Simple pattern matching (exact match for now)
        if (strcmp(entries[i].name, pattern) == 0) {
            // Print full path
            // If search_path is absolute, use it directly
            if (search_path[0] == '/') {
                if (strcmp(search_path, "/") != 0) {
                    serial_puts(search_path);
                    serial_puts("/");
                } else {
                    serial_puts("/");
                }
            } else {
                // Relative path - get current path and append
                char current_path[256];
                fs_get_current_path(current_path, 256);
                
                if (strcmp(current_path, "/") != 0) {
                    serial_puts(current_path);
                    serial_puts("/");
                }
                
                if (strcmp(search_path, ".") != 0) {
                    serial_puts(search_path);
                    serial_puts("/");
                }
            }
            serial_puts(entries[i].name);
            serial_puts("\n");
            found++;
        }
    }
    
    if (found == 0) {
        serial_puts("No files found matching '");
        serial_puts(pattern);
        serial_puts("' in ");
        serial_puts(search_path);
        serial_puts("\n");
    }
    
    return 0;
}

static int cmd_grep(int argc, char** argv) {
    if (argc < 3) {
        serial_puts("Usage: grep <pattern> <file>\n");
        serial_puts("Example: grep hello file.txt\n");
        return -1;
    }
    
    const char* pattern = argv[1];
    const char* filename = argv[2];
    
    // Open file
    int fd = fs_open(filename, FS_MODE_READ);
    if (fd < 0) {
        serial_puts("Error: Cannot open file: ");
        serial_puts(filename);
        serial_puts("\n");
        return -1;
    }
    
    // Read file into buffer
    uint32_t file_size = fs_size(fd);
    if (file_size == 0) {
        fs_close(fd);
        return 0; // Empty file
    }
    
    char* buffer = (char*)kmalloc(file_size + 1);
    if (!buffer) {
        serial_puts("Error: Out of memory\n");
        fs_close(fd);
        return -1;
    }
    
    int32_t bytes_read = fs_read(fd, buffer, file_size);
    fs_close(fd);
    
    if (bytes_read < 0 || (uint32_t)bytes_read != file_size) {
        serial_puts("Error: Cannot read file\n");
        kfree(buffer);
        return -1;
    }
    
    buffer[file_size] = '\0';
    
    // Simple pattern matching (exact substring search)
    int pattern_len = strlen(pattern);
    int found = 0;
    int line_num = 1;
    char* line_start = buffer;
    
    // Search for pattern
    for (int i = 0; i <= (int)file_size - pattern_len; i++) {
        // Check if we found the pattern
        if (memcmp(buffer + i, pattern, pattern_len) == 0) {
            // Find line number and print line
            // Count newlines before this position
            line_num = 1;
            line_start = buffer;
            for (int j = 0; j < i; j++) {
                if (buffer[j] == '\n') {
                    line_num++;
                    line_start = buffer + j + 1;
                }
            }
            
            // Find end of line
            char* line_end = buffer + i;
            while (*line_end != '\n' && *line_end != '\0' && line_end < buffer + file_size) {
                line_end++;
            }
            
            // Print line number and line
            serial_putdec(line_num);
            serial_puts(": ");
            
            // Print from line start to line end
            for (char* p = line_start; p < line_end && p < buffer + file_size; p++) {
                serial_putchar(*p);
            }
            serial_puts("\n");
            
            found++;
        }
    }
    
    if (found == 0) {
        serial_puts("Pattern not found\n");
    }
    
    kfree(buffer);
    return 0;
}

static int cmd_wc(int argc, char** argv) {
    if (argc < 2) {
        serial_puts("Usage: wc <file>\n");
        return -1;
    }
    
    const char* filename = argv[1];
    
    // Open file
    int fd = fs_open(filename, FS_MODE_READ);
    if (fd < 0) {
        serial_puts("Error: Cannot open file: ");
        serial_puts(filename);
        serial_puts("\n");
        return -1;
    }
    
    // Read file
    uint32_t file_size = fs_size(fd);
    char* buffer = (char*)kmalloc(file_size + 1);
    if (!buffer) {
        serial_puts("Error: Out of memory\n");
        fs_close(fd);
        return -1;
    }
    
    int32_t bytes_read = fs_read(fd, buffer, file_size);
    fs_close(fd);
    
    if (bytes_read < 0 || (uint32_t)bytes_read != file_size) {
        serial_puts("Error: Cannot read file\n");
        kfree(buffer);
        return -1;
    }
    
    buffer[file_size] = '\0';
    
    // Count lines, words, characters
    uint32_t lines = 0;
    uint32_t words = 0;
    uint32_t chars = file_size;
    int in_word = 0;
    
    for (uint32_t i = 0; i < file_size; i++) {
        if (buffer[i] == '\n') {
            lines++;
        }
        
        if (buffer[i] == ' ' || buffer[i] == '\t' || buffer[i] == '\n' || buffer[i] == '\r') {
            if (in_word) {
                words++;
                in_word = 0;
            }
        } else {
            in_word = 1;
        }
    }
    
    // Count last word if file doesn't end with whitespace
    if (in_word) {
        words++;
    }
    
    // Print results
    serial_putdec(lines);
    serial_puts(" ");
    serial_putdec(words);
    serial_puts(" ");
    serial_putdec(chars);
    serial_puts(" ");
    serial_puts(filename);
    serial_puts("\n");
    
    kfree(buffer);
    return 0;
}

static int cmd_head(int argc, char** argv) {
    if (argc < 2) {
        serial_puts("Usage: head <file> [lines]\n");
        serial_puts("Default: 10 lines\n");
        return -1;
    }
    
    const char* filename = argv[1];
    uint32_t num_lines = 10; // Default
    
    if (argc > 2) {
        // Parse number of lines (simple - just convert string to number)
        num_lines = 0;
        const char* num_str = argv[2];
        while (*num_str >= '0' && *num_str <= '9') {
            num_lines = num_lines * 10 + (*num_str - '0');
            num_str++;
        }
        if (num_lines == 0) num_lines = 10;
    }
    
    // Open file
    int fd = fs_open(filename, FS_MODE_READ);
    if (fd < 0) {
        serial_puts("Error: Cannot open file: ");
        serial_puts(filename);
        serial_puts("\n");
        return -1;
    }
    
    // Read file
    uint32_t file_size = fs_size(fd);
    char* buffer = (char*)kmalloc(file_size + 1);
    if (!buffer) {
        serial_puts("Error: Out of memory\n");
        fs_close(fd);
        return -1;
    }
    
    int32_t bytes_read = fs_read(fd, buffer, file_size);
    fs_close(fd);
    
    if (bytes_read < 0 || (uint32_t)bytes_read != file_size) {
        serial_puts("Error: Cannot read file\n");
        kfree(buffer);
        return -1;
    }
    
    buffer[file_size] = '\0';
    
    // Print first N lines
    uint32_t lines_printed = 0;
    for (uint32_t i = 0; i < file_size && lines_printed < num_lines; i++) {
        serial_putchar(buffer[i]);
        if (buffer[i] == '\n') {
            lines_printed++;
        }
    }
    
    kfree(buffer);
    return 0;
}

static int cmd_tail(int argc, char** argv) {
    if (argc < 2) {
        serial_puts("Usage: tail <file> [lines]\n");
        serial_puts("Default: 10 lines\n");
        return -1;
    }
    
    const char* filename = argv[1];
    uint32_t num_lines = 10; // Default
    
    if (argc > 2) {
        // Parse number of lines
        num_lines = 0;
        const char* num_str = argv[2];
        while (*num_str >= '0' && *num_str <= '9') {
            num_lines = num_lines * 10 + (*num_str - '0');
            num_str++;
        }
        if (num_lines == 0) num_lines = 10;
    }
    
    // Open file
    int fd = fs_open(filename, FS_MODE_READ);
    if (fd < 0) {
        serial_puts("Error: Cannot open file: ");
        serial_puts(filename);
        serial_puts("\n");
        return -1;
    }
    
    // Read file
    uint32_t file_size = fs_size(fd);
    char* buffer = (char*)kmalloc(file_size + 1);
    if (!buffer) {
        serial_puts("Error: Out of memory\n");
        fs_close(fd);
        return -1;
    }
    
    int32_t bytes_read = fs_read(fd, buffer, file_size);
    fs_close(fd);
    
    if (bytes_read < 0 || (uint32_t)bytes_read != file_size) {
        serial_puts("Error: Cannot read file\n");
        kfree(buffer);
        return -1;
    }
    
    buffer[file_size] = '\0';
    
    // Count total lines
    uint32_t total_lines = 0;
    for (uint32_t i = 0; i < file_size; i++) {
        if (buffer[i] == '\n') {
            total_lines++;
        }
    }
    
    // Find starting position (skip lines until we have num_lines left)
    uint32_t lines_to_skip = (total_lines > num_lines) ? (total_lines - num_lines) : 0;
    uint32_t lines_skipped = 0;
    uint32_t start_pos = 0;
    
    for (uint32_t i = 0; i < file_size; i++) {
        if (buffer[i] == '\n') {
            lines_skipped++;
            if (lines_skipped > lines_to_skip) {
                start_pos = i + 1;
                break;
            }
        }
    }
    
    // Print from start_pos to end
    for (uint32_t i = start_pos; i < file_size; i++) {
        serial_putchar(buffer[i]);
    }
    
    kfree(buffer);
    return 0;
}
