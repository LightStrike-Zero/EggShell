/**
 * @file terminal.c
 * @brief Terminal settings management
 *
 * This file contains functions to switch terminal modes and handle terminal-specific tasks.
 * 
 */

#include "terminal.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h> // contains struct for winsize. to store the size of the users terminal. 

struct termios original_terminal_input;

void make_raw_terminal() {
    struct termios raw_terminal_input_mode;
    if (tcgetattr(STDIN_FILENO, &original_terminal_input) == -1) {
        perror("tcgetattr");
        exit(1);
    }
    raw_terminal_input_mode = original_terminal_input;
    // Set raw mode
    raw_terminal_input_mode.c_lflag &= ~(ECHO | ICANON);
    raw_terminal_input_mode.c_cc[VMIN] = 1;
    raw_terminal_input_mode.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_terminal_input_mode) == -1) {
        perror("tcsetattr");
        exit(1);
    }
}

void restore_terminal()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal_input);
    // restores terminal to original settings for standard input
}


// do you want this @Lulu???
void get_terminal_size(int *rows, int *cols)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *rows = w.ws_row; 
    *cols = w.ws_col;
}