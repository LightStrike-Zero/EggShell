/**
 * @file builtins.c
 * @brief Implementation of built-in shell commands
 *
 * This file contains functions for built-in commands like cd, pwd, exit, etc.
 * 
 * @author 
 * @date 
 */

#include "builtins.h"
#include "definitions.h"
#include "terminal.h"

// System includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/limits.h> // check this, vs code was getting upset about not have the linux/limits

void change_directory(char *path) {
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

void cd(char *path)
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
    char cwd[PATH_MAX];  // PATH_MAX is defined in <limits.h>
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
        int len = strlen(new_hostname);
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

void clientConnect(const char *hostname)
{

 int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[255];


    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        error("Error opening socket.");
    }

    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "Error, no such host\n");
        close(sockfd);
        return;
    }

// configure server address structure
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *) &serv_addr.sin_addr.s_addr, (char *) server->h_addr, server->h_length);
    serv_addr.sin_port = htons(PORT);



    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        error("Connection failed.");
        close(sockfd);
        return;
    }

    // Enter username
       printf("Enter username: ");
    memset(buffer, 0, 255);
    fgets(buffer, 255, stdin);
    n = write(sockfd, buffer, strlen(buffer) - 1); // Send username
    if (n < 0) {
        perror("Error writing username to socket");
        close(sockfd);
        return;
    }

    // Enter password
    printf("Enter password: ");
    memset(buffer, 0, 255);
    fgets(buffer, 255, stdin);
    n = write(sockfd, buffer, strlen(buffer) - 1); // Send password
    if (n < 0) {
        perror("Error writing password to socket");
        close(sockfd);
        return;
    }

    memset(buffer, 0, 255);
    n = read(sockfd, buffer, 255);
    if (n < 0) {
        perror("Error reading login response");
    } else {
        printf("Server: %s\n", buffer);
    }

    close(sockfd);
}



void exit_shell() {
    restore_terminal();
    printf("Exiting shell...\n");
    exit(0);
}