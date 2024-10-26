#ifndef TERMINAL_H
#define TERMINAL_H

#include <termios.h>

extern struct termios original_terminal_input;

void make_raw_terminal();
void restore_terminal();
void get_terminal_size(int *rows, int *cols);

#endif // TERMINAL_H