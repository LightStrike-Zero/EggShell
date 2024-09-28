/**
 * @file simple_shell.c
 * @brief A basic UNIX shell implementation for ICT374
 *
 * This program implements a basic UNIX shell that reads user commands,
 * executes them by forking child processes, and handles the `exit` command to
 * terminate the shell. It also includes functionality to reclaim zombie
 * processes.
 *
 * @author Shaun Matthews & Louise Barjaktarevic
 * @date 25/09/2024
 */

/* Our includes */
#include "simple_shell.h"
#include "token.h"
#include "history.h"

/* System includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>




void read_command(char *command) {
    struct termios raw_terminal_input_mode;
    tcgetattr(STDIN_FILENO, &original_terminal_input); // Get current terminal attributes
    raw_terminal_input_mode = original_terminal_input; // copy attribs to raw term mode

    // set raw mode
    raw_terminal_input_mode.c_lflag &= ~(ECHO | ICANON); // Turn off echo and canonical mode
    raw_terminal_input_mode.c_cc[VMIN] = 1;  // Minimum number of characters to read
    raw_terminal_input_mode.c_cc[VTIME] = 0; // No timeout
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_terminal_input_mode); // set active terminal mode

    int index = 0;
    int history_pos = history_count; // Start at the end of history
    char current_command[MAX_COMMAND_LENGTH] = {0}; // To store current command when navigating history

    printf("%s", PS1);
    fflush(stdout);

    while (1) {
        char c;
        ssize_t nread = read(STDIN_FILENO, &c, 1);

        if (nread == -1 && errno != EAGAIN) {
            perror("read");
            break;
        } else if (nread == 0) {
            // EOF (Ctrl+D)
            printf("\n");
            exit(0);
        }

        if (c == '\n') {
            command[index] = '\0'; // Null-terminate the command
            printf("\n");
            break;
        } else if (c == BACKSPACE || c == 8) {
            if (index > 0) {
                index--;
                command[index] = '\0';
                // Move cursor back, overwrite the character with space, move cursor back again
                printf("\b \b");
                fflush(stdout);
            }
        } else if (c == ESCAPE) {
            // Escape sequence for special keys
            char seq[3];
            if (read(STDIN_FILENO, &seq[0], 1) == 0) break;
            if (read(STDIN_FILENO, &seq[1], 1) == 0) break;

            if (seq[0] == '[') {
                if (seq[1] == 'A') {
                    // Up arrow
                    if (history_count == 0) continue; // No history
                    if (history_pos > 0) history_pos--;
                    strcpy(current_command, history[history_pos % HISTORY_SIZE]);
                    // Clear current line
                    printf("\r\33[2K"); // Carriage return and clear line
                    printf("%s%s", PS1, current_command);
                    fflush(stdout);
                    strcpy(command, current_command);
                    index = strlen(command);
                } else if (seq[1] == 'B') {
                    // Down arrow
                    if (history_count == 0) continue; // No history
                    if (history_pos < history_count) history_pos++;
                    if (history_pos == history_count) {
                        // Clear command
                        current_command[0] = '\0';
                    } else {
                        strcpy(current_command, history[history_pos % HISTORY_SIZE]);
                    }
                    // Clear current line
                    printf("\r\33[2K");
                    printf("%s%s", PS1, current_command);
                    fflush(stdout);
                    strcpy(command, current_command);
                    index = strlen(command);
                }
                // Ignore other sequences
            }
        } else if (isprint(c)) {
            if (index < MAX_COMMAND_LENGTH - 1) {
                command[index++] = c;
                command[index] = '\0';
                printf("%c", c);
                fflush(stdout);
            }
        }
    }

    // Restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_terminal_input);

    // Add command to history if it's not empty
    if (strlen(command) > 0) {
        strcpy(history[history_count % HISTORY_SIZE], command);
        history_count++;
        history_index = history_count; // Reset history index to the end
    }
}

void restore_terminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal_input);
}

void pwd_recurse(Node *nodePtr)
{
    if (nodePtr == root)
    {
        return;
    }
    pwd_recurse(nodePtr->parent);
    printf("/%s", nodePtr->name); // will unravel path from cwd -> root, printing from root -> cwd!
}

void pwd()
{
    pwd_recurse(cwd); // recursive function, starts from cwd, cwd' parent, etc, etc.
    printf("\n");
}


void execute_command(char *command, int is_background) {
    char *args[MAX_ARGS];
    int num_tokens;

    // Handle empty commands
    if (strlen(command) == 0) {
        return;
    }

    // Tokenize the command
    num_tokens = tokenise(command, args);
    if (num_tokens < 0) {
        fprintf(stderr, "Error: Too many tokens\n");
        return;
    }

    // Handle built-in commands before forking
    if (strcmp(args[0], "exit") == 0) {
        restore_terminal();
        printf("Exiting shell...\n");
        exit(0);
    }
    else if (strcmp(args[0], "history") == 0) {
        show_history();
        return;
    }
    else if (strcmp(args[0], "pwd") == 0) {
        pwd();
        return;
    }
    // Add other built-in commands here...

    // Fork and execute external commands
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return;
    }
    if (pid == 0) {
        // Child process
        if (is_background) {
            setpgid(0, 0); // Detach from the controlling terminal
        }
        if (execvp(args[0], args) < 0) {
            perror("execvp");
            exit(1);
        }
    }
    else {
        // Parent process
        if (!is_background) {
            waitpid(pid, NULL, 0);
        } else {
            printf("[Background PID: %d]\n", pid);
        }
    }
}

void parse_commands(char *input_command) {
    char *commands[MAX_ARGS];
    char separators[MAX_ARGS];
    int num_commands = 0;

    // Copy input_command to a temporary buffer because strtok modifies the string
    char temp_input[MAX_COMMAND_LENGTH];
    strncpy(temp_input, input_command, MAX_COMMAND_LENGTH);
    temp_input[MAX_COMMAND_LENGTH - 1] = '\0';

    char *cmd = temp_input;
    while (cmd != NULL && num_commands < MAX_ARGS) {
        // Find next occurrence of ';' or '&'
        char *sep = strpbrk(cmd, ";&");
        if (sep != NULL) {
            separators[num_commands] = *sep;
            *sep = '\0';
            commands[num_commands++] = cmd;
            cmd = sep + 1;
        } else {
            // Last command without a following separator
            commands[num_commands++] = cmd;
            separators[num_commands - 1] = '\0';
            break;
        }
    }

    for (int i = 0; i < num_commands; i++) {
        char *command = commands[i];
        trim_whitespace(command);

        if (strlen(command) == 0) {
            continue;
        }

        // Determine if the command should run in the background
        int is_background = 0;
        if (separators[i] == '&') {
            is_background = 1;
        } else if (separators[i] == ';' || separators[i] == '\0') {
            is_background = 0;
        }

        // Check if the command ends with '&'
        size_t len = strlen(command);
        if (separators[i] == '\0' && len > 0 && command[len - 1] == '&') {
            is_background = 1;
            command[len - 1] = '\0';
            trim_whitespace(command);
        }

        // Execute the command
        execute_command(command, is_background);
    }
}

void setup_signal_handlers() {
    struct sigaction sa_int, sa_chld;

    // Setup for SIGINT
    sa_int.sa_handler = &handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("sigaction SIGINT");
        exit(1);
    }

    // Setup for SIGCHLD
    sa_chld.sa_handler = &handle_sigchld;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa_chld, NULL) == -1) {
        perror("sigaction SIGCHLD");
        exit(1);
    }
}

void handle_sigchld(int sig)
{
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        printf("[Process %d exited]\n", pid); // TODO remove this later????
    }
}

void handle_sigint(int sig) {
    restore_terminal();
    printf("\n");     
    exit(0);          
}


int main()
{
    char command[MAX_COMMAND_LENGTH];

    root = malloc(sizeof(Node));
    strcpy(root->name, ""); // root has no name.
    root->parent = NULL;
    // root set, with name and parent ("" and NULL).

    cwd = malloc(sizeof(Node));
    strcpy(cwd->name, "home");
    cwd->parent = root;

    // Set up signal handlers
    setup_signal_handlers();

    while (1) {
        // Read user input
        read_command(command);

        // Execute commands
        if (strlen(command) > 0) {
            parse_commands(command);
        }
    }

    return 0;
}