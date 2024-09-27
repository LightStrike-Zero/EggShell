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
        printf("%d %s\n", i + 1, history[i]); // Display history index + command
    }
}

void repeat_command_by_number(int command_number, char *command) {
    if (command_number > 0 && command_number <= history_count) {
        strcpy(command, history[command_number - 1]); // Copy the command from history
        printf("%s\n", command); // Print the repeated command
    } else {
        printf("No such command in history.\n");
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
}

void enable_raw_mode(struct termios *orig_termios) {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    *orig_termios = raw;

    raw.c_lflag &= ~(ICANON | ECHO); 
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode(struct termios *orig_termios) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, orig_termios); 
}

void handle_arrow_keys(char *command, const char *PS1) {  // PS1 passed as argument
    char seq[3];
    if (read(STDIN_FILENO, &seq[0], 1) == -1) return;
    if (read(STDIN_FILENO, &seq[1], 1) == -1) return;

    // Check for arrow key escape sequences
    if (seq[0] == '[') {
        if (seq[1] == 'A') {  // up arrow: previous command
            if (history_index > 0) {
                history_index--;
                printf("\r\033[K"); // clear the line
                printf("%s%s", PS1, history[history_index]); // prompt + previous command
                fflush(stdout); 
                strcpy(command, history[history_index]);
            }
        } else if (seq[1] == 'B') {  // down arrow: next command
            if (history_index < history_count - 1) {
                history_index++;
                printf("\r\033[K"); 
                printf("%s%s", PS1, history[history_index]);
                fflush(stdout); 
                strcpy(command, history[history_index]);
            } else {
                // If at the most recent command, clear the line
                history_index = history_count;
                printf("\r\033[K"); 
                printf("%s", PS1); 
                fflush(stdout); 
                command[0] = '\0'; 
            }
        }
    }
}
