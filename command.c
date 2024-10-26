/**
 * @file command_parser.c
 * @brief Command parsing functions
 *
 * This file contains functions to parse command strings into Command structures.
 * 
 */

#include "command.h"
#include "definitions.h"
#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>

void read_command(char *command)
{
    // put the terminal in non-canonical mode to gain raw control
    make_raw_terminal();
    int index = 0;
    printf("%s", PS1);
    fflush(stdout);
    while (1)
    {
        char character;
        ssize_t nread = read(STDIN_FILENO, &character, 1);
        if (nread == -1 && errno != EAGAIN)
        {
            perror("read");
            break;
        }
        else if (nread == 0)
        {
            // EOF (Ctrl+D)
            printf("\n");
            restore_terminal();
            exit(0);
        }
        if (character == '\n')
        {
            command[index] = '\0';
            printf("\n");
            break;
        }
        else if (character == BACKSPACE || character == 8)
        {
            if (index > 0)
            {
                index--;
                command[index] = '\0';
                printf("\b \b");
                fflush(stdout);
            }
        }
        else if (character == ESCAPE)
        {
            handle_history_navigation(command, &index, PS1);
        }
        else if (isprint(character))
        {
            if (index < MAX_COMMAND_LENGTH - 1)
            {
                command[index++] = character;
                command[index] = '\0';
                printf("%c", character);
                fflush(stdout);
            }
        }
    }
    // Restore original terminal settings
    restore_terminal();
    // Handle history repetition
    if (command[0] == '!') {
        if (command[1] == '\0') {
            // Repeat the last command
            if (history_count > 0) {
                repeat_command_by_number(history_count, command);
            } else {
                printf("No commands in history.\n");
                command[0] = '\0'; // Clear the command
            }
        } else if (isdigit(command[1])) {
            int cmd_num = atoi(&command[1]);
            repeat_command_by_number(cmd_num, command);
        } else {
            repeat_command_by_string(&command[1], command);
        }
    }

    // Add command to history if it's not empty and doesn't start with '!'
    if (strlen(command) > 0 && command[0] != '!') {
        add_to_history(command);
    }
}

// Initialize the Command struct
void init_command(Command *cmd) {
    cmd->original_command = NULL;
    cmd->command_name = NULL;
    memset(cmd->args, 0, sizeof(cmd->args));
    cmd->arg_count = 0;
    cmd->is_background = 0;
    cmd->input_redirection = NULL;
    cmd->output_redirection = NULL;
    cmd->error_redirection = NULL;
    cmd->append_output = 0;
    cmd->next = NULL;
}

// Free memory allocated in Command struct
void free_command(Command *cmd) {
    if (cmd == NULL) return;
    free(cmd->original_command);
    // Free each argument
    for (int i = 0; i < cmd->arg_count; i++) {
        free(cmd->args[i]);
    }
    free(cmd->input_redirection);
    free(cmd->output_redirection);
    free(cmd->error_redirection);
    free_command(cmd->next);
    free(cmd->next);
}

// Helper function to handle wildcard expansion
void expand_wildcards(char *token, Command *cmd) {
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));
    int ret = glob(token, GLOB_TILDE, NULL, &glob_result);
    if (ret == 0) {
        for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
            cmd->args[cmd->arg_count++] = strdup(glob_result.gl_pathv[i]);
        }
        globfree(&glob_result);
    } else {
        // No matches found, keep the token as is
        cmd->args[cmd->arg_count++] = strdup(token);
    }
}

// Parse individual command string into Command struct
int parse_command_string(char *input, Command *cmd) {

    init_command(cmd);
    cmd->original_command = strdup(input);

    char *tokens[MAX_TOKENS];
    int num_tokens = tokenise(input, tokens);
    int i = 0;

    int in_redirection = 0, out_redirection = 0, err_redirection = 0;

    while (i < num_tokens) {
        char *token = tokens[i];

        if (strcmp(token, "<") == 0) {
            in_redirection = 1;
        } else if (strcmp(token, ">") == 0) {
            out_redirection = 1;
            cmd->append_output = 0;
        } else if (strcmp(token, ">>") == 0) {
            out_redirection = 1;
            cmd->append_output = 1;
        } else if (strcmp(token, "2>") == 0) {
            err_redirection = 1;
        } else if (strcmp(token, "|") == 0) {
            // Handle pipeline
            if (i + 1 >= num_tokens) {
                fprintf(stderr, "Error: Expected command after '|'\n");
                return -1;
            }
            cmd->next = malloc(sizeof(Command));
            if (cmd->next == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
            // Construct remaining input for the next command
            char remaining_input[MAX_COMMAND_LENGTH] = "";
            for (int j = i + 1; j < num_tokens; j++) {
                strcat(remaining_input, tokens[j]);
                if (j < num_tokens - 1) {
                    strcat(remaining_input, " ");
                }
            }
            if (parse_command_string(remaining_input, cmd->next) != 0) {
                fprintf(stderr, "Error parsing piped command\n");
                return -1;
            }
            break;
        } else if (in_redirection) {
            cmd->input_redirection = strdup(token);
            in_redirection = 0;
        } else if (out_redirection) {
            cmd->output_redirection = strdup(token);
            out_redirection = 0;
        } else if (err_redirection) {
            cmd->error_redirection = strdup(token);
            err_redirection = 0;
        } else {
            // Normal argument
            cmd->args[cmd->arg_count++] = strdup(token);
        }
        i++;
    }
    cmd->args[cmd->arg_count] = NULL;

    if (cmd->arg_count > 0) {
        cmd->command_name = cmd->args[0];
    } else if (cmd->next == NULL) {
        // No command to execute
        return -1;
    }

    // Free tokens
    for (int k = 0; k < num_tokens; k++) {
        free(tokens[k]);
    }

    return 0;
}

void parse_commands(char *input_command) {
    char *commands_str[MAX_ARGS];
    char *separators[MAX_ARGS];
    int num_commands = 0;

    // Copy input_command to a temporary buffer because strtok modifies the string
    char temp_input[MAX_COMMAND_LENGTH];
    strncpy(temp_input, input_command, MAX_COMMAND_LENGTH);
    temp_input[MAX_COMMAND_LENGTH - 1] = '\0';

    char *cmd_str = temp_input;
    while (cmd_str != NULL && num_commands < MAX_ARGS) {
        // Find next occurrence of ';' or '&' not part of command arguments
        char *sep = strpbrk(cmd_str, ";&");
        if (sep != NULL) {
            separators[num_commands] = strndup(sep, 1);
            *sep = '\0';
            commands_str[num_commands++] = cmd_str;
            cmd_str = sep + 1;
        } else {
            // Last command without a following separator
            commands_str[num_commands++] = cmd_str;
            separators[num_commands - 1] = NULL;
            break;
        }
    }

    for (int i = 0; i < num_commands; i++) {
        Command cmd;
        char *command_str = commands_str[i];
        trim_whitespace(command_str);

        if (strlen(command_str) == 0) {
            continue;
        }

        // Initialize Command struct
        if (parse_command_string(command_str, &cmd) != 0) {
            fprintf(stderr, "Failed to parse command: %s\n", command_str);
            continue;
        }

        // Determine if the command should run in the background
        if (separators[i] != NULL && strcmp(separators[i], "&") == 0) {
            cmd.is_background = 1;
        }

        // Execute the command
        execute_command(&cmd);

        // Free allocated memory
        free_command(&cmd);
        free(separators[i]);
    }
}
