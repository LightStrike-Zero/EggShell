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

/**
 * @brief Executes a command by handling built-in commands, setting up pipes, and forking processes.
 * @param cmd Pointer to the Command structure representing the command to execute.
 */
void execute_command(Command *cmd);

/**
 * @brief Checks if a command is a built-in command and executes it if true.
 * @param cmd Command structure.
 * @return 1 if the command was a built-in, 0 otherwise.
 */
int handle_builtin_commands(Command *cmd);

/**
 * @brief Handles connection to the specified server.
 * @param cmd Command structure containing the hostname and optional port
 */
void handle_connect_command(Command *cmd);

/**
 * @brief Waits for all child processes
 * @param is_background Flag indicating if the command should run in the background
 * @param num_pipes The number of pipes
 */
void wait_for_children(int is_background, int num_pipes);

/**
 * @brief Sets redirections for a command if specified
 * @param cmd Command structure containing redirection information
 */
void handle_redirections(const Command *cmd);

/**
 * @brief the number of pipes in a command chain.
 * @param cmd starting Command structure
 * @return number of pipes found in the command chain
 */
int count_pipes(const Command *cmd);

/**
 * @brief sets up pipes for inter-process communication
 * @param pipefds array to store pipe file descriptors
 * @param num_pipes number of pipes to set up
 * @return 1 on successful setup 0 on failure
 */
int setup_pipes(int *pipefds, int num_pipes);

/**
 * @brief Executes a chain of commands connected by pipes
 * @param cmd starting Command structure
 * @param pipefds array of pipe file descriptors for inter-process communication
 * @param num_pipes The total number of pipes in the command chain
 */
void execute_with_pipes(Command *cmd, int *pipefds, int num_pipes);

/**
 * @brief Manages pipes by setting up file descriptors for input and output redirection
 * @param pipefds array of pipe file descriptors
 * @param cmd_index index of the current command in the pipeline
 * @param num_pipes  number of pipes
 * @param has_next indicates if there's a command in the chain
 */
void manage_pipes(int *pipefds, int cmd_index, int num_pipes, int has_next);

/**
 * @brief closes all open pipes in the file descriptor array
 * @param pipefds srray of pipe file descriptors
 * @param num_pipes number of pipes
 */
void close_pipes(int *pipefds, int num_pipes);

#endif
