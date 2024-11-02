//TODO add comments and header

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// #include <termios.h>

#define MAX_ARGS 1000
#define MAX_TOKENS 1000
#define MAX_TOKEN_LENGTH 1024
#define HISTORY_SIZE 100
#define MAX_COMMAND_LENGTH 1024
#define MAX_PROMPT_LENGTH 1024
#define MAX_PROMPT_LENGTH 1024

/* Defines for keys */
#define BACKSPACE 127
#define CTRL_D 4
#define ESCAPE 27   // ASCII "escape" character for arrow keys

/* Defines for the color types used in the shell */
#define MAGENTA        "\033[35m"  /* Standard Magenta */
#define BRIGHT_MAGENTA "\033[95m"
#define PINK            "\033[38;5;200m"  /* Pink */
#define BRIGHT_PINK     "\033[38;5;205m"  /* Bright Pink */
#define HOT_PINK      "\033[38;5;209m"  /* 256-color Hot Pink */
#define RED            "\033[31m"
#define GREEN          "\033[32m"
#define LIGHT_GREEN     "\033[38;5;118m"  /* Light Green */
#define YELLOW         "\033[33m"
#define BLUE           "\033[34m"  /* Standard Blue (Dark Blue) */
#define SKY_BLUE        "\033[38;5;117m"  /* 256-color Sky Blue */
#define BRIGHT_BLUE "\033[38;5;81m"
#define DARK_BLUE    "\033[38;5;19m"   /* 256-color Dark Blue */
#define NAVY            "\033[38;5;19m"   /* Navy (Dark Blue) */
#define TEAL            "\033[38;5;37m"   /* Teal */
#define CYAN           "\033[36m"
#define BRIGHT_ORANGE   "\033[38;5;214m"  /* Bright Orange */
#define DARK_ORANGE     "\033[38;5;202m"  /* Dark Orange */
#define RESET "\033[0m"


#define PORT 42010

extern char PS1[MAX_COMMAND_LENGTH];

#endif // DEFINITIONS_H