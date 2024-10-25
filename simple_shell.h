#ifndef SIMPLE_SHELL_H
#define SIMPLE_SHELL_H

#include "command.h"

#include <termios.h>

// #define MAX_COMMAND_LENGTH 1024
// max length of command
// #define MAX_ARGS 100
// maximum # of args per command

/* Defines for keys */
#define BACKSPACE 127
#define CTRL_D 4
#define ESCAPE 27   // ASCII "escape" character for arrow keys


/* defines for the color types used in the shell */
#define END_PINK   "\033[0m" 
#define PINK    "\033[1;35m"


extern char PS1[MAX_COMMAND_LENGTH];
// original command line prompt is set
// var name PS1 to mimic og Unix shell
extern struct termios original_terminal_input; // global for defualt terminal attributes

typedef struct Command Command;

void read_command(char *command);
void make_raw_terminal();
void cd(char *path);
void restore_terminal();
void pwd();
void man();
void execute_command(Command *cmd);
void parse_commands(char *input_command);
void setup_signal_handlers();
void handle_sigchld(int sig);
void handle_sigint(int sig);
void handle_sigquit(int sig);

#endif // SIMPLE_SHELL_H