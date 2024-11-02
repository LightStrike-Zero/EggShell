/**
 * @file builtins.h
 * @brief Header file for built-in shell commands.
 *
 * This file declares the functions used to implement basic shell commands,
 * such as directory navigation, prompt settings, and server connection.
 *
 * @author Shaun Matthews & Louise Barjaktarevic
 * @date 24/10/24
 */

#ifndef BUILTINS_H
#define BUILTINS_H

/* Function Declarations */
/**
 * @brief Changes the current working directory to the specified path
 * @param path The target directory path to change to
 */
void change_directory(const char *path);

/**
 * @brief Changes the current working directory to the specified path or home if none is provided
 * @param path The target directory path or NULL to go to the home directory
 */
void cd(const char *path);

/**
 * @brief Prints the current working directory
 */
void pwd();

/**
 * Connects to a remote server and facilitates data exchange.
 *
 * @param hostname The server's hostname or IP address.
 * @param port The server's port number.
 */
void connect_to_server(char *hostname, const int port);

/**
 * Relays data between stdin and the connected socket.
 *
 * @param sockfd The connected socket file descriptor.
 */
void relay_data(int sockfd);

/**
 * Creates a TCP socket and connects to the specified hostname and port.
 *
 * @param hostname The server's hostname or IP address.
 * @param port The server's port number.
 * @return The connected socket file descriptor, or -1 on failure.
 */
int create_and_connect_socket(const char *hostname, const int port);

/**
 * @brief Displays a manual of available commands
 */
void man();

/**
 * @brief Sets a new prompt for the shell
 * @param new_prompt The new prompt string to be displayed
 */
void set_prompt(const char *new_prompt);

/**
 * @brief Exits the shell program
 */
void exit_shell();

#endif // BUILTINS_H