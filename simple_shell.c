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
#include "token.h"

/* System includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 100

char PS1[MAX_COMMAND_LENGTH] = "[374-shell] $ ";

void read_command(char *command) {
    printf("%s", PS1);
    fflush(stdout);  
    if (fgets(command, MAX_COMMAND_LENGTH, stdin) == NULL) {
        perror("fgets");
        exit(1);
    }

    size_t len = strlen(command);
    if (len > 0 && command[len - 1] == '\n') {
        command[len - 1] = '\0';
    }
}

void execute_command(char *command) {
    char *args[MAX_ARGS];
    int num_tokens;

    // tokenise
    num_tokens = tokenise(command, args);
    if (num_tokens < 0) {
        fprintf(stderr, "Error: Too many tokens\n");
        return;
    }

    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

// to change hostname (sets PS1, which in bash is the hostname of the terminal)
    if (strcmp(args[0], "hostname") == 0) {
        if (num_tokens > 1) {
            snprintf(PS1, sizeof(PS1), "%s > ", args[1]); // Update PS1
            printf("Prompt changed to: %s\n", PS1);
        } else {
            printf("Usage: hostname <new_prompt>\n");
        }
        return;
    }
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        if (execvp(args[0], args) < 0) {
            perror("execvp");
            exit(1);
        }
    } else {
        wait(NULL);
    }
}



int main() 
    char command[MAX_COMMAND_LENGTH];

    while (1) {
        // user input
        read_command(command);

        // Execute
        if (strlen(command) > 0) {
            execute_command(command);
        }
    }

    return 0;
}