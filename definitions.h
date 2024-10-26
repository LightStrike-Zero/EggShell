#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// #include <termios.h>

#define MAX_ARGS 1000
#define MAX_TOKENS 1000
#define MAX_COMMAND_LENGTH 1024

/* Defines for keys */
#define BACKSPACE 127
#define CTRL_D 4
#define ESCAPE 27   // ASCII "escape" character for arrow keys

/* Defines for the color types used in the shell */
#define END_PINK   "\033[0m" 
#define PINK    "\033[1;35m"

extern char PS1[MAX_COMMAND_LENGTH];

#endif // DEFINITIONS_H