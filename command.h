// command.h
#ifndef COMMAND_H
#define COMMAND_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glob.h>

#define MAX_ARGS 1000
#define MAX_TOKENS 1000
#define MAX_COMMAND_LENGTH 1024

typedef struct Command {
    char *original_command;           // The original command string entered by the user
    char *command_name;               // The name of the command (e.g., "ls")
    char *args[MAX_ARGS];             // Array of arguments
    int arg_count;                    // Number of arguments
    int is_background;                // Flag indicating if the command should run in the background
    char *input_redirection;          // File for input redirection, if any
    char *output_redirection;         // File for output redirection, if any
    char *error_redirection;          // File for error redirection, if any
    int append_output;                // Flag to indicate if output should be appended (for >>)
    struct Command *next;             // Pointer to next command in a pipeline
} Command;

// Function declarations
void init_command(Command *cmd);
int parse_command_string(char *input, Command *cmd);
void free_command(Command *cmd);
void execute_command(Command *cmd);
void parse_commands(char *input_command);

#endif
