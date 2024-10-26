/**
 * @file execution.c
 * @brief Command execution
 *
 * This file contains the function to execute commands, handling pipelines, redirections, etc.
 * 
 */

#include "execution.h"
#include "builtins.h"
#include "simple_shell.h"
#include "signals.h"
#include "terminal.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

void execute_command(Command *cmd) {
    if (cmd == NULL || cmd->command_name == NULL) {
        return;
    }

    // Handle built-in commands
    if (strcmp(cmd->command_name, "exit") == 0) {
        restore_terminal();
        printf("Exiting shell...\n");
        exit_shell();
    } else if (strcmp(cmd->command_name, "history") == 0) {
        show_history();
        return;
    } else if (strcmp(cmd->command_name, "pwd") == 0) {
        pwd();
        return;
    } else if (strcmp(cmd->command_name, "man") == 0) {
        man();
        return;
    } else if (strcmp(cmd->command_name, "hostname") == 0) {
        change_hostname();
        return;
    } else if (strcmp(cmd->command_name, "cd") == 0) {
        if (cmd->arg_count > 1) {
            cd(cmd->args[1]); // cd with argument
        } else {
            cd(NULL); // cd without argument
        }
        return;
    } else if (strcmp(cmd->command_name, "prompt") == 0) {
        if (cmd->arg_count > 1) {
            snprintf(PS1, sizeof(PS1), "%s", cmd->args[1]);
        } else {
            fprintf(stderr, "Usage: prompt <new_prompt>\n");
        }
        return;
    }

    int num_pipes = 0;
    Command *current_cmd = cmd;
    while (current_cmd->next != NULL) {
        num_pipes++;
        current_cmd = current_cmd->next;
    }

    int pipefds[2 * num_pipes];
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("pipe");
            return;
        }
    }

    int pid;
    int cmd_index = 0;
    current_cmd = cmd;

    while (current_cmd != NULL) {
        pid = fork();
        if (pid == 0) {
            // Child process

            // Input redirection
            if (current_cmd->input_redirection) {
                freopen(current_cmd->input_redirection, "r", stdin);
            }

            // Output redirection
            if (current_cmd->output_redirection) {
                freopen(current_cmd->output_redirection, current_cmd->append_output ? "a" : "w", stdout);
            }

            // Error redirection
            if (current_cmd->error_redirection) {
                freopen(current_cmd->error_redirection, "w", stderr);
            }

            // If not the first command, read from previous pipe
            if (cmd_index != 0) {
                if (dup2(pipefds[(cmd_index - 1) * 2], STDIN_FILENO) < 0) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            // If not the last command, write to next pipe
            if (current_cmd->next != NULL) {
                if (dup2(pipefds[cmd_index * 2 + 1], STDOUT_FILENO) < 0) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            // Close all pipe fds
            for (int i = 0; i < 2 * num_pipes; i++) {
                close(pipefds[i]);
            }

            // Execute command
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

    // Close all pipe fds in parent
    for (int i = 0; i < 2 * num_pipes; i++) {
        close(pipefds[i]);
    }

    // Wait for all child processes
    if (!cmd->is_background) {
        for (int i = 0; i <= num_pipes; i++) {
            wait(NULL);
        }
    } else {
        printf("[Background PID: %d]\n", pid);
    }
}
