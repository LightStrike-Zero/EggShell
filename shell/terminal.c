/**
 * @file terminal.c
 * @brief Terminal settings management
 *
 * This file contains functions to switch terminal modes and handle terminal-specific tasks.
 * 
 */

/* Project Includes */
#include "terminal.h"

/* System Includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

/* Global Variables */
struct termios original_terminal_input;

/* Function Definitions */
void make_raw_terminal() {
    if (!isatty(STDIN_FILENO)) {
        // if not running in terminal (e.g., over a network socket), skip raw mode
        return;
    }

    struct termios raw_terminal_input_mode;
    if (tcgetattr(STDIN_FILENO, &original_terminal_input) == -1) {
        perror("tcgetattr");
        exit(1);
    }
    raw_terminal_input_mode = original_terminal_input;
    // set raw mode
    raw_terminal_input_mode.c_lflag &= ~(ECHO | ICANON);
    raw_terminal_input_mode.c_cc[VMIN] = 1;
    raw_terminal_input_mode.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_terminal_input_mode) == -1) {
        perror("tcsetattr");
        exit(1);
    }
}

void restore_terminal() {
    if (isatty(STDIN_FILENO)) {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal_input);
    }
}