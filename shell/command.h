//TODO add a proper header comment
// command.h

#ifndef COMMAND_H
#define COMMAND_H

/* Project Includes */
#include "definitions.h"

/* Global Variables */
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

/* Function Declarations */
//TODO add comments to the function declarations
// Function declarations
void read_command(char *command);
void init_command(Command *cmd);
int parse_command_string(const char *input, Command *cmd);
void free_command(const Command *cmd);
void execute_command(Command *cmd);
void parse_commands(const char *input_command);
void execute_command(Command *cmd);
int handle_builtin_commands(Command *cmd);
void handle_connect_command(Command *cmd);

void wait_for_children(int is_background, int num_pipes);
void handle_redirections(const Command *cmd);
int count_pipes(const Command *cmd);
int setup_pipes(int *pipefds, int num_pipes);
void execute_with_pipes(Command *cmd, int *pipefds, int num_pipes);
void manage_pipes(int *pipefds, int cmd_index, int num_pipes, int has_next);
void close_pipes(int *pipefds, int num_pipes);

#endif
