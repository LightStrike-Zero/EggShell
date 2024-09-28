#include "history.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

char history[HISTORY_SIZE][MAX_COMMAND_LENGTH];
int history_count = 0;
int history_index = -1; // Tracks the current position in the history when scrolling

void show_history() {
    for (int i = 0; i < history_count && i < HISTORY_SIZE; i++) {
        printf("%d %s\n", i + 1, history[i]);
    }
}

void add_to_history(const char *command) {
    strcpy(history[history_count % HISTORY_SIZE], command);
    history_count++;
    history_index = history_count; // Reset history index to the end
}

void handle_history_navigation(char *command, int *index, const char *PS1) {
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) == 0) return;
    if (read(STDIN_FILENO, &seq[1], 1) == 0) return;

    if (seq[0] == '[') {
        if (seq[1] == 'A') {
            // Up arrow
            if (history_count == 0) return; // No history
            if (history_index > 0) history_index--;
            else history_index = 0;
            strcpy(command, history[history_index % HISTORY_SIZE]);
        } else if (seq[1] == 'B') {
            // Down arrow
            if (history_count == 0) return; // No history
            if (history_index < history_count - 1) history_index++;
            else {
                // At the end of history
                history_index = history_count;
                command[0] = '\0';
            }
        } else {
            return; // Ignore other sequences
        }

        // Clear current line and display the command
        printf("\r\33[2K");
        printf("%s%s", PS1, command);
        fflush(stdout);
        *index = strlen(command);
    }
}

void repeat_command_by_number(int command_number, char *command) {
    if (command_number > 0 && command_number <= history_count) {
        char *retrieved_command = history[command_number - 1];
        if (retrieved_command[0] == '!') {
            printf("Cannot repeat a command that starts with '!'\n");
            command[0] = '\0';
            return;
        }
        strcpy(command, retrieved_command);
        printf("%s\n", command); 
    } else {
        printf("No such command in history.\n");
        command[0] = '\0';
    }
}

void repeat_command_by_string(char *prefix, char *command) {
    for (int i = history_count - 1; i >= 0; i--) {
        if (strncmp(history[i], prefix, strlen(prefix)) == 0) {
            strcpy(command, history[i]);
            printf("%s\n", command); 
            return;
        }
    }
    printf("No matching command found in history.\n");
    command[0] = '\0'; // Clear the command
}