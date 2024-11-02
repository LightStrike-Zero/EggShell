//TODO add header brief

#ifndef HISTORY_H
#define HISTORY_H

/* Project Includes */
#include "definitions.h"

/* System Includes */
#include <stddef.h>

/* Global Variables */
extern char history[HISTORY_SIZE][MAX_COMMAND_LENGTH];
extern int history_count;
extern int history_index;

/* Function Declarations */
//TODO add comments to the function declarations
void show_history();
void add_to_history(const char *command);
void handle_history_navigation(char *command, size_t *index, const char *PS1);
void repeat_command_by_number(const int command_number, char *command);
void repeat_command_by_string(const char *prefix, char *command);


#endif