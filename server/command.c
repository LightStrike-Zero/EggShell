/**
 * @file command.c
 * @brief Command parsing functions
 *
 * This file contains functions to parse command strings into Command structures.
 * 
 */

/* Project Includes */
#include "command.h"
#include "definitions.h"
#include "history.h"
#include "terminal.h"
#include "token.h"
#include "builtins.h"

/* System Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <glob.h>
#include <sys/wait.h>

/* End Includes */


void read_command(char *command) {
    // set terminal to raw mode for capturing input
    make_raw_terminal();
    
    size_t index = 0; // index for the command string
    size_t cursor_pos = 0; // cursor pos variable
    printf("%s", PS1); // print shell prompt
    fflush(stdout); // flush output buffer 

    while (1) {
        char character;
        const int line = read(STDIN_FILENO, &character, 1); // read 1 character from standard input 
        if (line == -1 && errno != EAGAIN) { // if error occurs while reading, print msg
            perror("Error reading input");
            restore_terminal(); // restore terminal settings 
            break;
        }
        if (line == 0) {
            // EOF (e.g., Ctrl+D)
            printf("\n");
            restore_terminal();
            exit(0);
        }

        if (character == '\n') {    // if user pressed enter 
            command[index] = '\0';  // null terminate the command string
            printf("\n");           // print newline
            break;
        }
        if (character == BACKSPACE || character == 8) {
            // handle backspace character 
            if (cursor_pos > 0 && index > 0) {
                // shift characters left from cursor position
                memmove(&command[cursor_pos - 1], &command[cursor_pos], index - cursor_pos);
                index--; // decrease command length
                cursor_pos--; // move cursor left
                command[index] = '\0'; // null terminate command string

                // move cursor back, clear to end, and reprint the rest of the command
                printf("\b");
                printf("\033[K"); // clear from cursor to end of line
                printf("%s", &command[cursor_pos]);

                // move cursor back to the correct position
                for (size_t i = cursor_pos; i < index; i++) {
                    printf("\033[D"); // move cursor left
                }
                fflush(stdout);
            }
        } else if (character == ESCAPE) {
            handle_history_navigation(command, &index, &cursor_pos, PS1);
        } else if (isprint(character)) {
            if (index < MAX_COMMAND_LENGTH - 1) {
                // insert character at cursor position
                memmove(&command[cursor_pos + 1], &command[cursor_pos], index - cursor_pos);
                command[cursor_pos] = character;
                index++;
                cursor_pos++;
                command[index] = '\0';

                // print the character and the rest of the command
                printf("\033[s"); // save cursor position
                printf("\033[K"); // clear from cursor -> end of line
                printf("%s", &command[cursor_pos - 1]);
                printf("\033[u"); // restore cursor position
                printf("%c", character);
                fflush(stdout);
            }
        }
    }

    restore_terminal();  // restore terminal settings at the end

    // handle history repetition if command starts with '!'
    if (command[0] == '!') {
        if (command[1] == '\0') {
            if (history_count > 0) {
                repeat_command_by_number(history_count, command);
            } else {
                printf("No commands in history.\n");
                command[0] = '\0';
            }
        } else if (isdigit(command[1])) {
            const int command_num = atoi(&command[1]);
            repeat_command_by_number(command_num, command);
        } else {
            repeat_command_by_string(&command[1], command);
        }
    }

    // add command to history if it's not empty and doesn't start with '!'
    if (strlen(command) > 0 && command[0] != '!') {
        add_to_history(command);
    }
}
// initialise the Command struct
void init_command(Command *cmd) {
    cmd->original_command = NULL;               // stores the original command string, set to NULL upon initialistion
    cmd->command_name = NULL;                   // main command to be executed
    memset(cmd->args, 0, sizeof(cmd->args));    // array of args for the command - set to 0 bc ensures all arg pointers initialised to NULL, preventing accidental access
    cmd->arg_count = 0;                         // number of args
    cmd->is_background = 0;                     // indicates if command should be run in the background
    cmd->input_redirection = NULL;              // file to redirect input from
    cmd->output_redirection = NULL;             // file to redirect output to 
    cmd->error_redirection = NULL;              // file to redirect error output to 
    cmd->append_output = 0;                     // indicates if output should be appended to the file 
    cmd->next = NULL;                           // pointer to the next command in a pipeline. 

}

// free memory allocated in Command struct
void free_command(const Command *cmd) {
    if (cmd == NULL) return;
    free(cmd->original_command);
    // free each argument recursively 
    for (int i = 0; i < cmd->arg_count; i++) {
        free(cmd->args[i]);
    }
    free(cmd->input_redirection);
    free(cmd->output_redirection);
    free(cmd->error_redirection);
    free_command(cmd->next);
    free(cmd->next);
}

// helper function to handle wildcard expansion
void expand_wildcards(const char *token, Command *cmd) {
    glob_t glob_output = {0};
    const int glob_result_code = glob(token, GLOB_TILDE, NULL, &glob_output);
    if (glob_result_code == 0) {
        for (size_t i = 0; i < glob_output.gl_pathc; ++i) {
            cmd->args[cmd->arg_count++] = strdup(glob_output.gl_pathv[i]);
        }
        globfree(&glob_output);
    } else {
        // no matches found, keep the token as is
        cmd->args[cmd->arg_count++] = strdup(token);
    }
}

// parse individual command string into Command struct
int parse_command_string(const char *input, Command *cmd) {

    init_command(cmd);
    cmd->original_command = strdup(input);

    char *tokens[MAX_TOKENS];
    const int num_tokens = tokenise(input, tokens);
    int i = 0;

    int in_redirection = 0, out_redirection = 0, err_redirection = 0;

    while (i < num_tokens) {
        const char *token = tokens[i];

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
            // handle pipeline
            if (i + 1 >= num_tokens) {
                fprintf(stderr, "Error: Expected command after '|'\n");
                return -1;
            }
            cmd->next = malloc(sizeof(Command));
            if (cmd->next == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
            // construct remaining input for the next command
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
            // setting flags for cmd
            cmd->input_redirection = strdup(token);
            in_redirection = 0;
        } else if (out_redirection) {
            cmd->output_redirection = strdup(token);
            out_redirection = 0;
        } else if (err_redirection) {
            cmd->error_redirection = strdup(token);
            err_redirection = 0;
        } else {
            // normal argument
            cmd->args[cmd->arg_count++] = strdup(token);
        }
        i++;
    }
    cmd->args[cmd->arg_count] = NULL;

    if (cmd->arg_count > 0) {
        cmd->command_name = cmd->args[0];
    } else if (cmd->next == NULL) {
        // no command to execute
        return -1;
    }

    // free tokens
    for (int k = 0; k < num_tokens; k++) {
        free(tokens[k]);
    }

    return 0;
}

void parse_commands(const char *input_command) {
    char *commands_str[MAX_ARGS];
    char *separators[MAX_ARGS];
    int num_commands = 0;

    // copy input_command to a temporary buffer because strtok modifies the string
    char temp_input[MAX_COMMAND_LENGTH];
    strncpy(temp_input, input_command, MAX_COMMAND_LENGTH);
    temp_input[MAX_COMMAND_LENGTH - 1] = '\0';

    char *cmd_str = temp_input;
    while (cmd_str != NULL && num_commands < MAX_ARGS) {
        // find next occurrence of ';' or '&' not part of command arguments
        char *sep = strpbrk(cmd_str, ";&");
        if (sep != NULL) {
            separators[num_commands] = strndup(sep, 1);
            *sep = '\0';
            commands_str[num_commands++] = cmd_str;
            cmd_str = sep + 1;
        } else {
            // last command without a following separator
            commands_str[num_commands++] = cmd_str;
            separators[num_commands - 1] = NULL;
            break;
        }
    }

    for (int i = 0; i < num_commands; i++) {
        Command cmd;
        char *command_str = commands_str[i];
        trim_whitespace(command_str); // trims all whitespace from command

        if (strlen(command_str) == 0) {
            continue;
        }

        // initialize Command struct
        if (parse_command_string(command_str, &cmd) != 0) {
            fprintf(stderr, "Failed to parse command: %s\n", command_str);
            continue;
        }

        // determine if the command should run in the background
        if (separators[i] != NULL && strcmp(separators[i], "&") == 0) {
            cmd.is_background = 1;
        }

        // execute the command
        execute_command(&cmd);

        // free allocated memory
        free_command(&cmd);
        free(separators[i]);
    }
}

void execute_command(Command *cmd) {
    if (cmd == NULL || cmd->command_name == NULL) {
        return;
    }

    if (handle_builtin_commands(cmd)) {
        return;
    }

    int num_pipes = count_pipes(cmd);
    int pipefds[2 * num_pipes];

    if (!setup_pipes(pipefds, num_pipes)) {
        return;
    }

    execute_with_pipes(cmd, pipefds, num_pipes);

    close_pipes(pipefds, num_pipes);
    wait_for_children(cmd->is_background, num_pipes);
}

int handle_builtin_commands(Command *cmd) {
    // string compares to check if input = a built in command
    if (strcmp(cmd->command_name, "exit") == 0) {
        exit_shell();
        return 1;
    } else if (strcmp(cmd->command_name, "history") == 0) {
        show_history();
        return 1;
    } else if (strcmp(cmd->command_name, "pwd") == 0) {
        pwd();
        return 1;
    } else if (strcmp(cmd->command_name, "man") == 0) {
        man();
        return 1;
    } else if (strcmp(cmd->command_name, "prompt") == 0) {
        set_prompt(cmd->args[1]);
        return 1;
    } else if (strcmp(cmd->command_name, "cd") == 0) {
        cd(cmd->arg_count > 1 ? cmd->args[1] : NULL);
        return 1;
    } else if (strcmp(cmd->command_name, "connect") == 0) {
        handle_connect_command(cmd);
        return 1;
    }
    return 0;
}

void handle_connect_command(Command *cmd) {
    if (cmd->arg_count == 3) {
        connect_to_server(cmd->args[1], atoi(cmd->args[2]));
    } else if (cmd->arg_count == 2) {
        connect_to_server(cmd->args[1], 40210);
    } else {
        fprintf(stderr, "Usage: connect <hostname> || connect <hostname> <port>\n");
    }
}

void handle_redirections(const Command *cmd) {
    if (cmd->input_redirection) {
        freopen(cmd->input_redirection, "r", stdin);
    }
    if (cmd->output_redirection) {
        freopen(cmd->output_redirection, cmd->append_output ? "a" : "w", stdout);
    }
    if (cmd->error_redirection) {
        freopen(cmd->error_redirection, "w", stderr);
    }
}

void wait_for_children(int is_background, int num_pipes) {
    if (!is_background) {
        for (int i = 0; i <= num_pipes; i++) {
            wait(NULL);
        }
    } else {
        printf("[Background PID: %d]\n", getpid());
    }
}

int count_pipes(const Command *cmd) {
    int num_pipes = 0;
    while (cmd->next != NULL) {
        num_pipes++;
        cmd = cmd->next;
    }
    return num_pipes;
}

int setup_pipes(int *pipefds, int num_pipes) {
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("pipe");
            return 0;
        }
    }
    return 1;
}

void execute_with_pipes(Command *cmd, int *pipefds, int num_pipes) {
    int pid, cmd_index = 0;
    const Command *current_cmd = cmd;

    while (current_cmd != NULL) {
        pid = fork();
        if (pid == 0) {
            handle_redirections(current_cmd);
            manage_pipes(pipefds, cmd_index, num_pipes, current_cmd->next != NULL);
            execvp(current_cmd->command_name, current_cmd->args);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("fork");
            return;
        }

        current_cmd = current_cmd->next;
        cmd_index++;
    }
}

void manage_pipes(int *pipefds, int cmd_index, int num_pipes, int has_next) {
    if (cmd_index != 0) {
        if (dup2(pipefds[(cmd_index - 1) * 2], STDIN_FILENO) < 0) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
    }
    if (has_next) {
        if (dup2(pipefds[cmd_index * 2 + 1], STDOUT_FILENO) < 0) {
            perror("dup2");
            exit(EXIT_FAILURE);
        }
    }
    close_pipes(pipefds, num_pipes);
}

void close_pipes(int *pipefds, int num_pipes) {
    for (int i = 0; i < 2 * num_pipes; i++) {
        close(pipefds[i]);
    }
}

