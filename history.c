#include "history.h"
#include "definitions.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

int history_count = 0;
CommandStack history_stack = {.top = -1}; // Initialize stack

// Push command onto the stack
void push_command(CommandStack *stack, const char *command) {
    if (stack->top < HISTORY_SIZE - 1) {
        stack->top++;
        strncpy(stack->commands[stack->top], command, MAX_COMMAND_LENGTH - 1);
        stack->commands[stack->top][MAX_COMMAND_LENGTH - 1] = '\0'; // Ensure null-termination
    } else {
        // If stack is full, shift commands up to make room for the new command
        for (int i = 1; i < HISTORY_SIZE; i++) {
            strcpy(stack->commands[i - 1], stack->commands[i]);
        }
        strncpy(stack->commands[HISTORY_SIZE - 1], command, MAX_COMMAND_LENGTH - 1);
        stack->commands[HISTORY_SIZE - 1][MAX_COMMAND_LENGTH - 1] = '\0'; // Null-terminate
    }
}

// Add command to history (push onto stack)
void add_to_history(const char *command) {
    push_command(&history_stack, command);
}

// Show all commands in the stack
void show_history(const CommandStack *stack) {
    for (int i = 0; i <= stack->top; i++) {
        printf("%d %s\n", i + 1, stack->commands[i]);
    }
}

// Retrieve command from stack based on the index
const char* get_command(const CommandStack *stack, int index) {
    if (index >= 0 && index <= stack->top) {
        return stack->commands[index];
    }
    return NULL; // Return NULL if the index is out of bounds
}

// Handle up/down arrow navigation through history
void handle_history_navigation(CommandStack *stack, char *command, int *index, const char *PS1) {
    static int current_index = -1; // Keeps track of current navigation position in stack
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) == 0) return;
    if (read(STDIN_FILENO, &seq[1], 1) == 0) return;

    if (seq[0] == '[') {
        if (seq[1] == 'A') { // Up arrow
            if (stack->top == -1) return; // No history
            if (current_index > 0) current_index--;
            else current_index = 0;
            strcpy(command, stack->commands[current_index]);
        } else if (seq[1] == 'B') { // Down arrow
            if (stack->top == -1) return; // No history
            if (current_index < stack->top) current_index++;
            else {
                current_index = stack->top + 1;
                command[0] = '\0';
            }
        } else {
            return;
        }

        // Clear current line and display the command
        printf("\r\33[2K");
        printf("%s%s", PS1, command);
        fflush(stdout);
        *index = strlen(command);
    }
}

// Repeat command by number
void repeat_command_by_number(const CommandStack *stack, int command_number, char *command) {
    int index = command_number - 1;
    const char *retrieved_command = get_command(stack, index);
    if (retrieved_command) {
        if (retrieved_command[0] == '!') {
            printf("Cannot repeat a command that starts with '!'\n");
            command[0] = '\0';
            return;
        }
        strcpy(command, retrieved_command);
        printf("%s\n", command); // Echo the command
    } else {
        printf("No such command in history.\n");
        command[0] = '\0';
    }
}

// Repeat command by string prefix
void repeat_command_by_string(const CommandStack *stack, const char *prefix, char *command) {
    for (int i = stack->top; i >= 0; i--) {
        if (strncmp(stack->commands[i], prefix, strlen(prefix)) == 0) {
            strcpy(command, stack->commands[i]);
            printf("%s\n", command);
            return;
        }
    }
    printf("No matching command found in history.\n");
    command[0] = '\0';
}
