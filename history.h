#ifndef HISTORY_H
#define HISTORY_H

#include <termios.h>

#define HISTORY_SIZE 100 
#define MAX_COMMAND_LENGTH 1024 

extern char history[HISTORY_SIZE][MAX_COMMAND_LENGTH];
extern int history_count;
extern int history_index;

void show_history();
void repeat_command_by_number(int command_number, char *command);
void repeat_command_by_string(char *prefix, char *command);
void enable_raw_mode(struct termios *orig_termios);
void disable_raw_mode(struct termios *orig_termios);
void handle_arrow_keys(char *command, const char *PS1);

#endif