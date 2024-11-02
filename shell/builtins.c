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
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/limits.h>

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

void change_hostname()
{
    char new_hostname[MAX_COMMAND_LENGTH]; // buffer for the new hostname
    printf("Enter new hostname: ");
    if (fgets(new_hostname, sizeof(new_hostname), stdin) != NULL)
    { // Remove the newline character, if present
        const size_t len = strlen(new_hostname);
        if (len > 0 && new_hostname[len - 1] == '\n') {
            new_hostname[len - 1] = '\0';
        }
        if (strlen(new_hostname) > 0 && strlen(new_hostname) < MAX_COMMAND_LENGTH)
        {
            snprintf(PS1, sizeof(PS1), "[%s] $", new_hostname);
            printf("Hostname has been changed.\n");
        }
        else
        {
            printf("Invalid hostname.\n");
        }
    }
    else
    {
        perror("fgets");
    }
    fflush(stdout);
}

void man()
{
    // displays user manual.
    // list of available commands
    printf("This is Eggshell's user manual.\n"
           "Available commands:\n"
           "Usage: <hostname> $ <command>\n"
          PINK "pwd -     Prints the working directory.\n"
           "history - Use up and down arrow keys to toggle through command history.\n"
           "exit -    Exit shell. Bye Bye.\n"END_PINK); 
}

void error(const char *msg) {
    perror(msg);
    exit(0);
}

void connect_to_server(char *hostname, const int port) {
    struct sockaddr_in server_addr;
    char buffer[1024];

    const int socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_file_descriptor < 0) {
        perror("Socket creation failed");
        return;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, hostname, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        close(socket_file_descriptor);
        return;
    }

    if (connect(socket_file_descriptor, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(socket_file_descriptor);
        return;
    }

    printf("Connected to %s:%d\n", hostname, port);

    char username[256];
    char password[256];

    printf("Username: ");
    fflush(stdout);
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = '\0';

    printf("Password: ");
    fflush(stdout);
    fgets(password, sizeof(password), stdin);
    printf("\n");
    password[strcspn(password, "\n")] = '\0';

    snprintf(buffer, sizeof(buffer), "%s %s", username, password);
    send(socket_file_descriptor, buffer, strlen(buffer), 0);

    ssize_t bytes_received = recv(socket_file_descriptor, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0) {
        perror("Error receiving data from server");
        close(socket_file_descriptor);
        return;
    }

    buffer[bytes_received] = '\0';
    printf("Server response: %s\n", buffer);

    if (strstr(buffer, "Authentication successful") != NULL) {
        printf("Logged in successfully.\n");
    } else {
        printf("Authentication failed.\n");
        close(socket_file_descriptor);
        return;
    }

    while (1) {
        printf("Remote shell> ");
        fflush(stdout);

        char command[1024];
        if (fgets(command, sizeof(command), stdin) == NULL) {
            printf("\nDisconnecting...\n");
            break;
        }

        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "exit") == 0 || strcmp(command, "logout") == 0 || strcmp(command, "quit") == 0) {
            printf("Disconnecting...\n");
            break;
        }

        if (send(socket_file_descriptor, command, strlen(command), 0) < 0) {
            perror("Error sending command to server");
            break;
        }

        while ((bytes_received = recv(socket_file_descriptor, buffer, sizeof(buffer) - 1, 0)) > 0) {
            buffer[bytes_received] = '\0';
            printf("%s", buffer);
            if (strstr(buffer, "%")) {
                break;
            }
        }

        send(socket_file_descriptor, "READY", 5, 0);
    }

    close(socket_file_descriptor);
}

void exit_shell() {
    restore_terminal();
    printf("Exiting shell...\n");
    exit(0);
}