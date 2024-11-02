
//TODO add comments and header

/* Project Includes */
#include "history.h"

/* System Includes */
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char history[HISTORY_SIZE][MAX_COMMAND_LENGTH];
// defined in history.h, allocated here.
int history_count = 0;
int history_index = -1; // Tracks the current position in the history when scrolling
int history_start = 0; // Index of the oldest command in history

void show_history() {
    int start = (history_count > HISTORY_SIZE) ? history_start : 0;
    int count = (history_count > HISTORY_SIZE) ? HISTORY_SIZE : history_count;

    for (int i = 0; i < count; i++) {
        int pos = (start + i) % HISTORY_SIZE;
        printf("%d %s\n", start + i + 1, history[pos]);
    }
}

void add_to_history(const char *command) {
    if (command == NULL || strlen(command) == 0) return; // Ignore empty commands

    int pos = history_count % HISTORY_SIZE;
    strncpy(history[pos], command, MAX_COMMAND_LENGTH - 1);
    history[pos][MAX_COMMAND_LENGTH - 1] = '\0'; // Ensure null-termination
    history_count++;

    if (history_count > HISTORY_SIZE) {
        history_start = (history_start + 1) % HISTORY_SIZE;
    }

    history_index = history_count > HISTORY_SIZE ? (history_start + HISTORY_SIZE) : history_count; // Reset history index to the end
}

// Add cursor_pos as a separate variable
void handle_history_navigation(char *command, size_t *index, size_t *cursor_pos, const char *PS1) {
    char seq[3];

    // Read the next two characters after ESC
    if (read(STDIN_FILENO, &seq[0], 1) == 0) return;
    if (read(STDIN_FILENO, &seq[1], 1) == 0) return;

    if (seq[0] == '[') {
        if (seq[1] == 'A') {
            // Up arrow
            if (history_count == 0) return; // No history available

            history_index--;

            // Determine the minimum valid index based on history size
            int min_index = (history_count > HISTORY_SIZE) ? history_start : 0;

            // Wrap around if history_index goes below min_index
            if (history_index < min_index) {
                history_index = (history_count > HISTORY_SIZE) ? (history_start + HISTORY_SIZE - 1) % HISTORY_SIZE : history_count - 1;
            }

            // Calculate the correct position in the circular buffer
            int pos = history_index % HISTORY_SIZE;
            strcpy(command, history[pos]);

            // Update cursor position to end of command
            *cursor_pos = strlen(command);

            // Clear current line and display the command
            printf("\r\33[2K"); // Clear entire line
            printf("%s%s", PS1, command);
            fflush(stdout);
            *index = strlen(command);
        }
        else if (seq[1] == 'B') {
            // Down arrow
            if (history_count == 0) return; // No history available

            history_index++;

            // Determine the maximum valid index based on history size
            int max_index = history_count;

            // Wrap around if history_index exceeds max_index
            if (history_index >= max_index) {
                history_index = (history_count > HISTORY_SIZE) ? history_start : 0;
            }

            if (history_index < history_count) {
                int pos = history_index % HISTORY_SIZE;
                strcpy(command, history[pos]);
            }
            else {
                // At the end of history, clear the command line
                command[0] = '\0';
            }

            // Update cursor position
            *cursor_pos = strlen(command);

            // Clear current line and display the command
            printf("\r\33[2K"); // Clear entire line
            printf("%s%s", PS1, command);
            fflush(stdout);
            *index = strlen(command);
        }
        else if (seq[1] == 'C') {
            // Right arrow
            if (*cursor_pos < strlen(command)) {
                printf("\033[C"); // Move cursor right
                (*cursor_pos)++;
                fflush(stdout);
            }
        }
        else if (seq[1] == 'D') {
            // Left arrow
            if (*cursor_pos > 0) {
                printf("\033[D"); // Move cursor left
                (*cursor_pos)--;
                fflush(stdout);
            }
        }
        else {
            return; // Ignore other sequences
        }
    }
}


void repeat_command_by_number(const int command_number, char *command) {
    if (command_number <= 0 || command_number > history_count) {
        printf("No such command in history.\n");
        command[0] = '\0';
        return;
    }

    int pos;
    if (history_count > HISTORY_SIZE) {
        pos = (history_start + (command_number - 1)) % HISTORY_SIZE;
    }
    else {
        pos = command_number - 1;
    }

    const char *retrieved_command = history[pos];
    if (retrieved_command[0] == '!') {
        printf("Cannot repeat a command that starts with '!'\n");
        command[0] = '\0';
        return;
    }
    strcpy(command, retrieved_command);
    printf("%s\n", command); // Echo the command
}

void repeat_command_by_string(const char *prefix, char *command) {
    if (prefix == NULL || strlen(prefix) == 0) {
        printf("Prefix cannot be empty.\n");
        command[0] = '\0';
        return;
    }

    int max_history = (history_count > HISTORY_SIZE) ? HISTORY_SIZE : history_count;
    for (int i = 0; i < max_history; i++) {
        int pos = (history_start + history_count - 1 - i) % HISTORY_SIZE;
        if (strncmp(history[pos], prefix, strlen(prefix)) == 0) {
            strcpy(command, history[pos]);
            printf("%s\n", command); 
            return;
        }
    }
    printf("No matching command found in history.\n");
    command[0] = '\0'; // Clear the command
}