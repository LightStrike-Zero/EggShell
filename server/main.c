/**
 * @file server.c
 * @brief PTY-Based Client-Server Shell Implementation
 *
 * This server listens for incoming client connections, authenticates them,
 * spawns a shell within a PTY for each authenticated client, and facilitates
 * bidirectional communication between the client and the shell.
 *
 * Supports both custom clients and standard clients like Telnet.
 *
 * Author: [Your Name]
 * Date: [Date]
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <pty.h>
#include <utmp.h>
#include <termios.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096
#define USERNAME_LENGTH 30
#define PASSWORD_LENGTH 12
#define PORT 40210
#define MAX_CLIENTS 100

// Authentication structure
struct UserTable {
    char username[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];
} userTable = {"test", "test"};

// Function prototypes
int setup_server_socket(int port);
int authenticate(int client_fd);
void handle_client(int client_fd);
ssize_t strip_cr(char *buffer, ssize_t nbytes);
void sigchld_handler(int signum);

// Main function
int main()
{
    printf("PTY-Based Server Version: 0.27\n");

    // Set up the SIGCHLD handler to reap zombie processes
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; // Restart interrupted system calls

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // Initialize server socket
    int server_fd = setup_server_socket(PORT);

    // Array to keep track of client PIDs (optional, for management)
    pid_t client_pids[MAX_CLIENTS];
    memset(client_pids, 0, sizeof(client_pids));

    // Main server loop
    while (1)
    {
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);

        // Accept a new client connection
        int client_fd = accept(server_fd, (struct sockaddr *)&cli_addr, &clilen);
        if (client_fd < 0)
        {
            perror("Error on accept");
            continue;
        }

        printf("Connection from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

        // Fork a new process to handle the client
        pid_t pid = fork();
        if (pid == 0)
        {
            // Child process
            close(server_fd); // Close the listening socket in the child
            handle_client(client_fd); // Handle client communication
            exit(EXIT_SUCCESS); // Terminate child after handling client
        }
        else if (pid > 0)
        {
            // Parent process
            close(client_fd); // Close client socket in parent
            // Optionally, store client PIDs if needed
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_pids[i] == 0)
                {
                    client_pids[i] = pid;
                    break;
                }
            }
        }
        else
        {
            // Fork failed
            perror("Fork failed");
            close(client_fd);
        }
    }

    // Cleanup (unreachable in this code)
    close(server_fd);
    return 0;
}

/**
 * @brief Set up the server socket to listen on the specified port.
 *
 * @param port The port number to listen on.
 * @return The file descriptor for the listening socket.
 */
int setup_server_socket(int port)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    // Create socket (IPv4, TCP)
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error opening socket");
        exit(EXIT_FAILURE);
    }

    // Allow address reuse
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Configure server address structure
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET; // IPv4
    serv_addr.sin_addr.s_addr = INADDR_ANY; // Bind to all interfaces
    serv_addr.sin_port = htons(port); // Host to network byte order

    // Bind socket to the specified port
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error on binding");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Start listening with a backlog of 5
    if (listen(sockfd, 5) < 0)
    {
        perror("Error on listen");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);
    return sockfd;
}

/**
 * @brief Strip carriage return characters from the buffer.
 *
 * @param buffer The input buffer.
 * @param nbytes Number of bytes in the buffer.
 * @return The new length of the buffer after stripping.
 */
ssize_t strip_cr(char *buffer, ssize_t nbytes)
{
    ssize_t j = 0;
    for (ssize_t i = 0; i < nbytes; i++)
    {
        if (buffer[i] != '\r')
        {
            buffer[j++] = buffer[i];
        }
    }
    buffer[j] = '\0'; // Null-terminate for safe printing
    return j;
}

/**
 * @brief Authenticate the client by reading username and password.
 *
 * @param client_fd The client socket file descriptor.
 * @return 1 if authentication is successful, 0 otherwise.
 */
int authenticate(int client_fd)
{
    char buffer[USERNAME_LENGTH + PASSWORD_LENGTH + 3]; // +2 for space and '\n', +1 for '\0'
    int n = read(client_fd, buffer, sizeof(buffer) - 1); // Read into buffer
    if (n <= 0)
    {
        perror("Error reading from client during authentication");
        return 0;
    }

    buffer[n] = '\0'; // Null-terminate to ensure it's a proper string

    // Strip '\r' from buffer
    ssize_t clean_nbytes = strip_cr(buffer, n);
    buffer[clean_nbytes] = '\0'; // Ensure null-termination after stripping

    // Debugging: Print the stripped input
    printf("Authentication input after strip_cr: \"%s\"\n", buffer);

    // Parse username and password from buffer
    char received_username[USERNAME_LENGTH], received_password[PASSWORD_LENGTH];
    if (sscanf(buffer, "%29s %11s", received_username, received_password) != 2)
    {
        const char *invalid_format = "Invalid authentication format. Use: <username> <password>\n";
        write(client_fd, invalid_format, strlen(invalid_format));
        printf("Invalid authentication format received.\n");
        return 0;
    }

    // Check against the user table
    if (strcmp(received_username, userTable.username) == 0 &&
        strcmp(received_password, userTable.password) == 0)
    {
        const char *success = "Authentication successful\n";
        write(client_fd, success, strlen(success));
        printf("User \"%s\" authenticated successfully.\n", received_username); // Debugging
        return 1; // Authentication success
    }
    else
    {
        const char *failure = "Authentication failed\n";
        write(client_fd, failure, strlen(failure));
        printf("User \"%s\" failed to authenticate.\n", received_username); // Debugging
        return 0; // Authentication failed
    }
}

/**
 * @brief Handle communication between the client and the shell within a PTY.
 *
 * @param client_fd The client socket file descriptor.
 */
void handle_client(int client_fd)
{
    // Authenticate the client first
    if (!authenticate(client_fd))
    {
        close(client_fd);
        exit(EXIT_FAILURE); // Terminate child process if authentication fails
    }

    // Set the PATH environment variable to ensure the shell can locate commands
    if (setenv("PATH", "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin", 1) != 0)
    {
        perror("setenv failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // Allocate a PTY and fork the shell
    int master_fd;
    pid_t pid = forkpty(&master_fd, NULL, NULL, NULL);
    if (pid < 0)
    {
        perror("forkpty failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        // Child process: execute bash in interactive mode
        execlp("/bin/bash", "bash", "-i", "--noprofile", "--norc", NULL);
        // If execlp returns, an error occurred
        perror("execlp failed: No such file or directory");
        exit(EXIT_FAILURE);
    }

    // Parent process: communicate between client_fd and master_fd
    fd_set read_fds;
    int max_fd = (client_fd > master_fd) ? client_fd : master_fd;
    char buffer[BUFFER_SIZE];
    ssize_t nbytes;

    printf("Shell process started with PID %d\n", pid);

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(client_fd, &read_fds);
        FD_SET(master_fd, &read_fds);

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (activity < 0)
        {
            if (errno == EINTR)
                continue; // Interrupted by signal, restart select
            perror("select error");
            break;
        }

        // Data from client to shell
        if (FD_ISSET(client_fd, &read_fds))
        {
            nbytes = read(client_fd, buffer, sizeof(buffer));
            if (nbytes > 0)
            {
                // Strip '\r' if present (handles Telnet's CRLF)
                ssize_t clean_nbytes = strip_cr(buffer, nbytes);
                buffer[clean_nbytes] = '\0';

                // Debugging: Log the command received
                printf("Received from client: \"%s\"\n", buffer);

                // Optional: Handle special commands like "exit" or "quit"
                if (strncmp(buffer, "exit", 4) == 0 ||
                    strncmp(buffer, "quit", 4) == 0 ||
                    strncmp(buffer, "logout", 6) == 0)
                {
                    const char *disconnect_msg = "Disconnecting...\n";
                    write(client_fd, disconnect_msg, strlen(disconnect_msg));
                    printf("Received termination command from client.\n");
                    break;
                }

                // Write the received data to the shell's PTY
                ssize_t total_written = 0;
                while (total_written < clean_nbytes)
                {
                    ssize_t bytes_written = write(master_fd, buffer + total_written, clean_nbytes - total_written);
                    if (bytes_written <= 0)
                    {
                        perror("write to master_fd failed");
                        break;
                    }
                    total_written += bytes_written;
                }
            }
            else if (nbytes == 0)
            {
                // Client disconnected
                printf("Client disconnected.\n");
                break;
            }
            else
            {
                perror("read from client_fd failed");
                break;
            }
        }

        // Data from shell to client
        if (FD_ISSET(master_fd, &read_fds))
        {
            nbytes = read(master_fd, buffer, sizeof(buffer));
            if (nbytes > 0)
            {
                // Write the data to the client
                ssize_t total_written = 0;
                while (total_written < nbytes)
                {
                    ssize_t bytes_written = write(client_fd, buffer + total_written, nbytes - total_written);
                    if (bytes_written <= 0)
                    {
                        perror("write to client_fd failed");
                        break;
                    }
                    total_written += bytes_written;
                }

                // Optional: Log the output sent to client
                printf("Sent to client: \"%.*s\"\n", (int)nbytes, buffer);
            }
            else if (nbytes == 0)
            {
                // Shell closed
                printf("Shell closed the connection.\n");
                break;
            }
            else
            {
                perror("read from master_fd failed");
                break;
            }
        }
    }

    // Clean up
    close(master_fd);
    close(client_fd);
    // Optionally, terminate the shell process if it's still running
    kill(pid, SIGKILL);
    waitpid(pid, NULL, 0);
    printf("Client handler terminated.\n");
}

/**
 * @brief Signal handler to reap zombie processes.
 *
 * @param signum The signal number.
 */
void sigchld_handler(int signum)
{
    // Save and restore errno to avoid side effects
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
    errno = saved_errno;
}
