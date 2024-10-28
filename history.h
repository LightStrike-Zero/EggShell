#ifndef HISTORY_H
#define HISTORY_H

#define MAX_COMMAND_LENGTH 1024
#define HISTORY_SIZE 10

// Command stack structure
typedef struct {
    char commands[HISTORY_SIZE][MAX_COMMAND_LENGTH];
    int top; // Points to the top of the stack
} CommandStack;

// Functions to manage command history
void push_command(CommandStack *stack, const char *command);
void show_history(const CommandStack *stack);
const char* get_command(const CommandStack *stack, int index);
void handle_history_navigation(CommandStack *stack, char *command, int *index, const char *PS1);
void repeat_command_by_number(const CommandStack *stack, int command_number, char *command);
void repeat_command_by_string(const CommandStack *stack, const char *prefix, char *command);

#endif // HISTORY_H
