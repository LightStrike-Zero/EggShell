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
#include "history.h"

/* System includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 100



// original command line prompt is set
// var name PS1 to mimic og Unix shell 
char PS1[MAX_COMMAND_LENGTH] = "[374-shell] $ "; 

typedef struct Node { // typedef Node - each Node has a parent (unless root), and name
    struct Node *parent;
    char name[100]; // arbitrary number.. idk
} Node; 

Node *cwd; // will be replaced with custom made cwd command. 
Node *root; // root node!

void read_command(char *command) {
    struct termios orig_termios;
    enable_raw_mode(&orig_termios);

    int index = 0;
    printf("%s", PS1);
    fflush(stdout);

    while (1) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1) break;

        if (c == '\n') {
            command[index] = '\0'; // Null-terminate the command
            printf("\n");
            break;
        } else if (c == 127) { // Backspace
            if (index > 0) {
                index--;
                printf("\b \b"); // Erase the last character
            }
        } else if (c == '\033') { // Arrow keys start with an escape sequence
            handle_arrow_keys(command, PS1);
            index = strlen(command); // Update index to match the current command length
        } else {
            // Add character to command buffer
            if (index < MAX_COMMAND_LENGTH - 1) {
                command[index] = c;
                index++;
                write(STDOUT_FILENO, &c, 1); // Echo the character
            }
        }
    }

    disable_raw_mode(&orig_termios);

    // Add command to history if it's not empty
    if (strlen(command) > 0) {
        // Add the command to history
        strcpy(history[history_count % HISTORY_SIZE], command);
        history_count++;
        history_index = history_count; // Reset history index to the end
    }
}

void pwd_recurse(Node *nodePtr) {
    if (nodePtr == root) {
        return;
    }
    pwd_recurse(nodePtr->parent);
    printf("/%s", nodePtr->name); // will unravel path from cwd -> root, printing from root -> cwd!
}

void pwd() {
    pwd_recurse(cwd); // recursive function, starts from cwd, cwd' parent, etc, etc.
    printf("\n");
}

void execute_command(char *command) {
    char *args[MAX_ARGS];
    int num_tokens;

    // Check for the history command
    if (strcmp(command, "history") == 0) {
        show_history();
        return;
    }

    // Check for !<number> to repeat a command by number
    if (command[0] == '!' && isdigit(command[1])) {
        int command_number = atoi(&command[1]);
        repeat_command_by_number(command_number, command);
    }

    // Check for !<string> to repeat a command by string
    else if (command[0] == '!' && isalpha(command[1])) {
        repeat_command_by_string(&command[1], command);
    }

    // Tokenize the command
    num_tokens = tokenise(command, args);
    if (num_tokens < 0) {
        fprintf(stderr, "Error: Too many tokens\n");
        return;
    }

    // Handle exit command
    if (strcmp(args[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    // Change hostname
    if (strcmp(args[0], "hostname") == 0) {
        if (num_tokens > 1) {
            snprintf(PS1, sizeof(PS1), "%s > ", args[1]); // Update PS1
            printf("Prompt changed to: %s\n", PS1);
        } else {
            printf("Usage: hostname <new_prompt>\n");
        }
        return;
    }

    // Handle pwd command
    if (strcmp(args[0], "pwd") == 0) {
        pwd();
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

    // Add the command to history if it's not a history-related command
    if (history_count < HISTORY_SIZE) {
        strcpy(history[history_count], command);
        history_count++;
    }
}



int main() {
    char command[MAX_COMMAND_LENGTH];

    root = malloc(sizeof(Node));
    strcpy(root->name, ""); // root has no name. 
    root->parent = NULL; 
    // root set, with name and parent ("" and NULL).

    cwd = malloc(sizeof(Node));
    strcpy(cwd->name, "home");
    cwd->parent = root;

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