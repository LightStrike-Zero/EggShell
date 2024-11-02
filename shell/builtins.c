/**
 * @file builtins.c
 * @brief Implementation of built-in shell commands
 *
 *TODO is this all relevant?
 * This file contains functions for built-in commands like cd, pwd, exit, etc.
 * 
 * @author 
 * @date 
 */

/* Project Includes */
#include "builtins.h"
#include "definitions.h"
#include "terminal.h"

/* System Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for read, write, and close
#include <sys/types.h> // definitions for pid_t, size_t
#include <sys/socket.h> // networks comms - incl. definitions for address families, socket types, etc.
#include <linux/limits.h>
#include <sys/select.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
/* End Includes */

void change_directory(const char *path) {
    if (chdir(path) < 0) {
        perror("chdir failed");
    } else {
        // Get and print the new current directory after the change
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Changed directory to: %s\n", cwd);
        } else {
            perror("getcwd failed");
        }
    }
}

void cd(const char *path)
{
    if (path == NULL || strcmp(path, "") == 0) 
    {
        // Change to home directory if no argument is given
        const char *home_dir = getenv("HOME");
        if (home_dir == NULL) 
        {
            fprintf(stderr, "cd: HOME not set\n");
        } 
        else if (chdir(home_dir) < 0) 
        {
            perror("cd");
        }
    } 
    else if (strcmp(path, "..") == 0) 
    {
        // Move up one directory
        if (chdir("..") < 0) 
        {
            perror("cd");
        }
    } 
    else 
    {
        // Move to the specified directory
        if (chdir(path) < 0) 
        {
            perror("cd");
        }
    }


    
}

void pwd()
{
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("getcwd failed");
    }
}

void set_prompt(const char *new_prompt) {
    // Buffer to hold the processed prompt
    char processed_prompt[MAX_PROMPT_LENGTH];

    if (new_prompt == NULL) {
        fprintf(stderr, "Error: new_prompt is NULL.\n");
        return;
    }

    // Copy the input to processed_prompt with safety
    strncpy(processed_prompt, new_prompt, sizeof(processed_prompt) - 1);
    processed_prompt[sizeof(processed_prompt) - 1] = '\0';  // Ensure null-termination

    // Remove any newline characters from the input
    size_t len = strlen(processed_prompt);
    if (len > 0 && processed_prompt[len - 1] == '\n') {
        processed_prompt[len - 1] = '\0';
        len--;
    }
    // Check if the prompt length exceeds the maximum allowed length
    if (len > MAX_PROMPT_LENGTH - 3) {
        fprintf(stderr, "Error: Prompt is too long. Maximum length is %d characters.\n", MAX_PROMPT_LENGTH - 3);
        return;
    }

    // Format the PS1 variable with the new prompt
    const int written = snprintf(PS1, sizeof(PS1), "[%s] %%", processed_prompt);
    if (written < 0) {
        fprintf(stderr, "Error: Failed to set the prompt.\n");
        return;
    } else if ((size_t)written >= sizeof(PS1)) {
        fprintf(stderr, "Warning: Prompt was truncated.\n");
    }
    fflush(stdout);
}

void man() {
    // Displays the user manual with descriptions of available commands and features.
    printf("Eggshell User Manual\n");
    printf("This shell provides the following commands and features:\n\n");

    // Usage and prompt configuration
    printf("Usage:\n");
    printf("\t\t<prompt>\t$ <command>\n");
    printf("  Use the command prompt to execute commands and access shell features.\n\n");

    printf("Available Commands:\n");

    // Built-in commands
    printf("\tprompt\n");

    printf("\tpwd\n");

    printf("\tcd\n");

    printf("\thistory\n");

    printf("\texit\n");

    printf("End of manual.\n");
}

void error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void connect_to_server(char *hostname, const int port) {
    int sockfd;

    // Create and connect the socket
    sockfd = create_and_connect_socket(hostname, port);
    if (sockfd == -1) {
        fprintf(stderr, "Failed to establish connection to %s:%d\n", hostname, port);
        return;
    }

    printf("Connected to %s:%d\n", hostname, port);
    printf("Type your commands below. Press Ctrl+D to disconnect.\n");

    // Relay data between stdin and socket
    relay_data(sockfd);

    printf("Connection closed.\n");
}

void relay_data(int sockfd) {
    fd_set read_fds;
    int max_fd = (sockfd > STDIN_FILENO) ? sockfd : STDIN_FILENO;
    char buffer[4096];
    int n;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        FD_SET(sockfd, &read_fds);

        // Wait for data on either stdin or socket
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            break;
        }

        // Check if data is available on stdin
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            n = read(STDIN_FILENO, buffer, sizeof(buffer));
            if (n < 0) {
                perror("read from stdin");
                break;
            } else if (n == 0) {
                // EOF (Ctrl+D)
                printf("\nDisconnected from server.\n");
                break;
            }

            // Send data to server
            if (write(sockfd, buffer, n) != n) {
                perror("write to socket");
                break;
            }
        }

        // Check if data is available on the socket
        if (FD_ISSET(sockfd, &read_fds)) {
            n = read(sockfd, buffer, sizeof(buffer));
            if (n < 0) {
                perror("read from socket");
                break;
            } else if (n == 0) {
                // Server closed connection
                printf("\nServer closed the connection.\n");
                break;
            }

            // Write data to stdout
            if (write(STDOUT_FILENO, buffer, n) != n) {
                perror("write to stdout");
                break;
            }
        }
    }

    // Cleanup
    close(sockfd);
}

int create_and_connect_socket(const char *hostname, const int port) {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char port_str[6]; // Max port number is 65535

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;      // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP

    snprintf(port_str, sizeof(port_str), "%d", port);

    if ((rv = getaddrinfo(hostname, port_str, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    // Loop through all the results and connect to the first possible
    for (p = servinfo; p != NULL; p = p->ai_next) {
        // Create socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }

        // Connect to server
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("connect");
            continue;
        }

        break; // Successfully connected
    }

    if (p == NULL) {
        fprintf(stderr, "Failed to connect to %s:%d\n", hostname, port);
        freeaddrinfo(servinfo);
        return -1;
    }

    freeaddrinfo(servinfo); // All done with this structure

    return sockfd;
}

void exit_shell() {
    restore_terminal();
    printf("Exiting shell...\n");
    exit(0);
}