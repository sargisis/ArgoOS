// Shell (Command Line Interface)
// Interactive command processor

#ifndef SHELL_H
#define SHELL_H

#include "types.h"

#define SHELL_MAX_LINE_LENGTH 256
#define SHELL_MAX_ARGS 16
#define SHELL_PROMPT "FlowDay-OS> "

// Command handler function type
typedef int (*command_handler_t)(int argc, char** argv);

// Command structure
struct shell_command {
    const char* name;
    const char* description;
    command_handler_t handler;
};

// Initialize shell
void shell_init(void);

// Run shell (main loop)
void shell_run(void);

// Process a command line
int shell_process_command(const char* line);

// Register a command
void shell_register_command(const char* name, const char* description, command_handler_t handler);

// Get command by name
struct shell_command* shell_get_command(const char* name);

#endif // SHELL_H
