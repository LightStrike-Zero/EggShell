/**
 * @file history.h
 * @brief Header file for managing command history
 * 
 * @author Shaun Matthews & Louise Barjaktarevic
 * @date 04/10/24
 */


#ifndef HISTORY_H
#define HISTORY_H

/* Project Includes */
#include "definitions.h"

/* System Includes */
#include <stddef.h>

/* Global Variables */
extern char history[HISTORY_SIZE][MAX_COMMAND_LENGTH];  /**< Array to store command history */
extern int history_count;                               /**< Total count of commands in history */
extern int history_index;                               /**< Index for current position in history during navigation */
extern int history_start;                               /**< Index of the oldest command in history */

/* Function Declarations */
/**
 * @brief Displays the stored command history.
 */
void show_history();

/**
 * @brief Adds a command to the history array.
 * @param command The command string to add to history.
 */
void add_to_history(const char *command);

/**
 * @brief Navigates through command history, allowing the user to scroll up and down.
 * @param command Buffer to store the selected command from history.
 * @param index Pointer to the current command index.
 * @param cursor_pos Pointer to the cursor position within the command string.
 * @param PS1 The shell prompt to display before the command.
 */
void handle_history_navigation(char *command, size_t *index, size_t *cursor_pos, const char *PS1);

/**
 * @brief Repeats a command from history based on its numerical position.
 * @param command_number The position of the command to repeat.
 * @param command Buffer to store the repeated command.
 */
void repeat_command_by_number(const int command_number, char *command);

/**
 * @brief Repeats the most recent command from history that starts with a specific prefix.
 * @param prefix The prefix to search for in the command history.
 * @param command Buffer to store the repeated command.
 */
void repeat_command_by_string(const char *prefix, char *command);




#endif