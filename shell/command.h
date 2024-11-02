/**
 * @file command.h
 * @brief Header file for command parsing and execution.
 *
 * This file defines the Command structure and declares functions for parsing,
 * initialising, and executing commands.
 * 
 * @author Shaun Matthews & Louise Barjaktarevic
 * @date 09/10/24
 */


#ifndef COMMAND_H
#define COMMAND_H

/* Project Includes */
#include "definitions.h"

/**
 * @struct Command
 * @brief Represents a command entered by the user with its associated data
 *
 */
typedef struct Command {
    char *original_command;           /**< The original command string entered by the user */
    char *command_name;               /**< The name of the command */
    char *args[MAX_ARGS];             /**< Array of arguments */
    int arg_count;                    /**< Number of arguments */
    int is_background;                /**< Flag indicating if the command should run in the background */
    char *input_redirection;          /**< File for input redirection */
    char *output_redirection;         /**< File for output redirection */
    char *error_redirection;          /**< File for error redirection */
    int append_output;                /**< Flag to indicate if output should be appended (for >>) */
    struct Command *next;             /**< Pointer to next command in a pipeline */
} Command;

/**
 * @brief Reads a command from the user input
 * @param command The buffer to store the command string
 */
void read_command(char *command);

/**
 * @brief Initializes a Command structure, setting its fields to default values
 * @param cmd Pointer to the Command structure to initialise
 */
void init_command(Command *cmd);

/**
 * @brief Parses a command string into a Command structure
 * @param input The command string to parse
 * @param cmd Pointer to the Command structure to populate
 * @return 0 on success, -1 on failure
 */
int parse_command_string(const char *input, Command *cmd);

/**
 * @brief Frees memory allocated within a Command structure
 * @param cmd Pointer to the Command structure to free
 */
void free_command(const Command *cmd);

/**
 * @brief Executes a given command
 * @param cmd Pointer to the Command structure representing the command to execute.
 */
void execute_command(Command *cmd);

/**
 * @brief Parses multiple commands separated by delimiters
 * @param input_command The input command string containing multiple commands
 */
void parse_commands(const char *input_command);

#endif
