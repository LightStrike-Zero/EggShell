//TODO add file header comments here

#ifndef TERMINAL_H
#define TERMINAL_H

/* Project Includes */
#include <termios.h>


/* Global Variables */
extern struct termios original_terminal_input;

/* Function Declarations */
//TODO add comments to the function declarations
void make_raw_terminal();
void restore_terminal();
void get_terminal_size(int *rows, int *cols);

#endif // TERMINAL_H