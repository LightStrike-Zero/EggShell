#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <errno.h>    // Corrected from <cerrno>
#include <signal.h>   // Added for signal handling
#include <sys/wait.h> // Added for waitpid and WNOHANG

#define BUFFER_SIZE 1024

#define USERNAME_LENGTH 30
#define PASSWORD_LENGTH 12
#define PORT 40210

struct ClientInfo
{
    int client_socket;
    pid_t pid;
};

struct UserTable
{
    char username[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];
} userTable = {"test", "test"};

struct ClientInfo client_list[100]; // Fixed size for simplicity
int client_count = 0;               // Track the number of clients

// Function prototypes
int setup_server_socket(int port);
int authenticate(int client_fd);
void handle_client(int client_fd);
ssize_t strip_cr(char *buffer, ssize_t nbytes);
void sigchld_handler(int signum);

int main()
{
    // Set up the SIGCHLD handler to reap zombie processes
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; // Restart interrupted system calls

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    // Step 1: Initialize server socket
    int server_fd = setup_server_socket(PORT);

    // Main server loop
    while (1)
    {
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);

        // Step 2: Accept a new client connection
        int client_fd = accept(server_fd, (struct sockaddr *)&cli_addr, &clilen);
        if (client_fd < 0)
        {
            perror("Error on accept");
            continue;
        }

        // Step 3: Fork a new process for each client
        pid_t pid = fork();
        if (pid == 0)
        {                             // Child process
            close(server_fd);         // Close unused server socket in child
            handle_client(client_fd); // Handle the client
            exit(0);                  // End child process after handling client
        }
        else if (pid > 0)
        {
            if (client_count < 100)
            {
                client_list[client_count].client_socket = client_fd;
                client_list[client_count].pid = pid;
                client_count++;
            }
            else
            {
                perror("Max client limit reached.");
                close(client_fd); // Close the new client socket if limit reached
            }
            close(client_fd); // Close unused client socket in parent
        }
        else
        {
            perror("Fork failed");
            close(client_fd);
        }
    }

    // Cleanup
    close(server_fd);
    return 0;
}

int setup_server_socket(int port)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    // Open socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Error opening socket");
        exit(1);
    }

    // Configure server address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    // Bind socket to port
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Error on binding");
        close(sockfd);
        exit(1);
    }

    // Start listening on the socket
    if (listen(sockfd, 5) < 0)
    {
        perror("Error on listen");
        close(sockfd);
        exit(1);
    }

    printf("Server listening on port %d\n", port);

    // Return the server socket descriptor to main
    return sockfd;
}

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
    printf("After strip_cr: \"%s\"\n", buffer); // Debugging statement
    return j;
}

int authenticate(int client_fd)
{
    char buffer[USERNAME_LENGTH + PASSWORD_LENGTH + 1];  // Additional byte for separating username and password
    int n = read(client_fd, buffer, sizeof(buffer) - 1); // Read into buffer
    if (n <= 0)
    {
        perror("Error reading from client");
        return 0;
    }

    buffer[n] = '\0'; // Null-terminate to ensure it’s a proper string

    // Strip '\r' from buffer
    ssize_t clean_nbytes = strip_cr(buffer, n);
    buffer[clean_nbytes] = '\0'; // Ensure null-termination after stripping

    // Debugging: Print the stripped input
    printf("Authentication input after strip_cr: \"%s\"\n", buffer);

    // Parse username and password from buffer
    char received_username[USERNAME_LENGTH], received_password[PASSWORD_LENGTH];
    sscanf(buffer, "%29s %11s", received_username, received_password);

    // Check against the user table
    if (strcmp(received_username, userTable.username) == 0 &&
        strcmp(received_password, userTable.password) == 0)
    {
        write(client_fd, "Authentication successful\n", strlen("Authentication successful\n"));
        printf("User \"%s\" authenticated successfully.\n", received_username); // Debugging
        return 1; // Authentication success
    }
    else
    {
        write(client_fd, "Authentication failed\n", strlen("Authentication failed\n"));
        printf("User \"%s\" failed to authenticate.\n", received_username); // Debugging
        return 0; // Authentication failed
    }
}

void handle_client(int client_fd)
{
    // Authenticate the client first
    if (!authenticate(client_fd))
    {
        close(client_fd);
        exit(0); // Terminate child process if authentication fails
    }

    int shell_to_server[2]; // Pipe from shell's stdout/stderr to server
    int server_to_shell[2]; // Pipe from server to shell's stdin

    // Create pipes for communication with the shell
    if (pipe(shell_to_server) == -1 || pipe(server_to_shell) == -1)
    {
        perror("Pipe creation failed");
        close(client_fd);
        exit(1);
    }

    pid_t pid = fork();
    if (pid == 0)
    { // Child process (shell)
        // Redirect shell's stdin, stdout, and stderr to the pipes
        if (dup2(server_to_shell[0], STDIN_FILENO) == -1 ||
            dup2(shell_to_server[1], STDOUT_FILENO) == -1 ||
            dup2(shell_to_server[1], STDERR_FILENO) == -1)
        {
            perror("dup2 failed");
            exit(1);
        }

        // **Implementing Number 10: Close unused ends in child**
        close(server_to_shell[1]); // Child doesn't write to server_to_shell
        close(shell_to_server[0]); // Child doesn't read from shell_to_server
        close(client_fd);          // Child doesn't need client's socket

        // Execute the shell in non-interactive mode
        execlp("/bin/bash", "bash", "--noprofile", "--norc", NULL);
        perror("Failed to execute shell");
        exit(1);
    }
    else if (pid > 0)
    { // Parent process (server handling client)
        // **Implementing Number 10: Close unused ends in parent**
        close(server_to_shell[0]); // Parent doesn't read from server_to_shell
        close(shell_to_server[1]); // Parent doesn't write to shell_to_server

        fd_set read_fds;
        int max_fd = (client_fd > shell_to_server[0]) ? client_fd : shell_to_server[0];
        char buffer[BUFFER_SIZE];
        ssize_t nbytes;

        while (1)
        {
            FD_ZERO(&read_fds);
            FD_SET(client_fd, &read_fds);
            FD_SET(shell_to_server[0], &read_fds);

            // Wait for data on either the client socket or the shell output
            int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
            if (activity < 0 && errno != EINTR)
            {
                perror("Select error");
                break;
            }

            // **Implementing Number 8: Handle partial reads/writes from client to shell**
            if (FD_ISSET(client_fd, &read_fds))
            {
                nbytes = read(client_fd, buffer, sizeof(buffer));
                if (nbytes <= 0)
                {
                    if (nbytes == 0)
                    {
                        // Client disconnected
                        printf("Client disconnected.\n");
                    }
                    else
                    {
                        perror("Read error from client");
                    }
                    break;
                }

                // Strip '\r' from buffer
                ssize_t clean_nbytes = strip_cr(buffer, nbytes);
                buffer[clean_nbytes] = '\0'; // Ensure null-termination

                // Debugging: Print the command being sent to the shell
                printf("Sending to shell: \"%s\"\n", buffer);

                // Check for termination command
                if (strcmp(buffer, "Bye bye") == 0)
                {
                    write(client_fd, "Disconnecting...\n", strlen("Disconnecting...\n"));
                    printf("Received termination command from client.\n");
                    break;
                }

                // Skip empty commands
                if (clean_nbytes == 0)
                {
                    printf("Received empty command. Skipping.\n");
                    continue;
                }

                // Write cleaned data to the shell's stdin
                ssize_t total_written = 0;
                while (total_written < clean_nbytes)
                {
                    ssize_t bytes_written = write(server_to_shell[1], buffer + total_written, clean_nbytes - total_written);
                    if (bytes_written <= 0)
                    {
                        perror("Write error to shell");
                        break;
                    }
                    total_written += bytes_written;
                }
            }

            // **Implementing Number 8: Handle partial reads/writes from shell to client**
            if (FD_ISSET(shell_to_server[0], &read_fds))
            {
                nbytes = read(shell_to_server[0], buffer, sizeof(buffer));
                if (nbytes <= 0)
                {
                    if (nbytes == 0)
                    {
                        // Shell closed
                        printf("Shell process closed.\n");
                    }
                    else
                    {
                        perror("Read error from shell");
                    }
                    break;
                }

                // Debugging: Print the data received from the shell
                printf("Received from shell: \"%.*s\"\n", (int)nbytes, buffer);

                // Write shell output to the client
                ssize_t total_written = 0;
                while (total_written < nbytes)
                {
                    ssize_t bytes_written = write(client_fd, buffer + total_written, nbytes - total_written);
                    if (bytes_written <= 0)
                    {
                        perror("Write error to client");
                        break;
                    }
                    total_written += bytes_written;
                }
            }
        }

        // Close all open descriptors
        close(server_to_shell[1]);
        close(shell_to_server[0]);
        close(client_fd);
    }
    else
    {
        perror("Fork failed");
        close(client_fd);
        exit(1);
    }
}

// Signal handler to reap zombie processes
void sigchld_handler(int signum)
{
    // Save and restore errno to avoid side effects
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
    errno = saved_errno;
}
