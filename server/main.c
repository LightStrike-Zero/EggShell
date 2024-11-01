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
#include <pty.h>     // For PTY functions
#include <utmp.h>
#include <termios.h>
#include <fcntl.h>

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
    buffer[j] = '\0';                           // Null-terminate for safe printing
    // printf("After strip_cr: \"%s\"\n", buffer); // Debugging statement
    return j;
}

int authenticate(int client_fd)
{
    char buffer[USERNAME_LENGTH + PASSWORD_LENGTH + 2];  // Additional bytes for separating username and password
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

    // Parse username and password from buffer
    char received_username[USERNAME_LENGTH], received_password[PASSWORD_LENGTH];
    sscanf(buffer, "%29s %11s", received_username, received_password);

    // Check against the user table
    if (strcmp(received_username, userTable.username) == 0 &&
        strcmp(received_password, userTable.password) == 0)
    {
        write(client_fd, "Authentication successful\n", strlen("Authentication successful\n"));
        printf("User \"%s\" authenticated successfully.\n", received_username); // Debugging
        return 1;                                                               // Authentication success
    }
    else
    {
        write(client_fd, "Authentication failed\n", strlen("Authentication failed\n"));
        printf("User \"%s\" failed to authenticate.\n", received_username); // Debugging
        return 0;                                                           // Authentication failed
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

    int master_fd; // File descriptor for the master side of the PTY
    pid_t shell_pid;

    // Fork a new process with a PTY
    shell_pid = forkpty(&master_fd, NULL, NULL, NULL);
    if (shell_pid == -1)
    {
        perror("forkpty failed");
        close(client_fd);
        exit(1);
    }

    if (shell_pid == 0)
    { // Child process (shell)
        // Execute the shell in interactive mode
        execlp("/bin/bash", "bash", "--noprofile", "--norc", "-i", NULL);
        perror("Failed to execute shell");
        exit(1);
    }
    else
    { // Parent process (server handling client)
        fd_set read_fds;
        int max_fd = (client_fd > master_fd) ? client_fd : master_fd;
        char buffer[BUFFER_SIZE];
        ssize_t nbytes;

        // Set master_fd and client_fd to non-blocking mode (optional, can help prevent blocking)
        // fcntl(master_fd, F_SETFL, O_NONBLOCK);
        // fcntl(client_fd, F_SETFL, O_NONBLOCK);

        while (1)
        {
            FD_ZERO(&read_fds);
            FD_SET(client_fd, &read_fds);
            FD_SET(master_fd, &read_fds);

            // Wait for data on either the client socket or the shell PTY
            int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
            if (activity < 0 && errno != EINTR)
            {
                perror("Select error");
                break;
            }

            // Data from client to shell
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

                // Write data to the shell's PTY
                ssize_t total_written = 0;
                while (total_written < nbytes)
                {
                    ssize_t bytes_written = write(master_fd, buffer + total_written, nbytes - total_written);
                    if (bytes_written <= 0)
                    {
                        perror("Write error to shell PTY");
                        break;
                    }
                    total_written += bytes_written;
                }
            }

            // Data from shell to client
            if (FD_ISSET(master_fd, &read_fds))
            {
                nbytes = read(master_fd, buffer, sizeof(buffer));
                if (nbytes <= 0)
                {
                    if (nbytes == 0)
                    {
                        // Shell closed
                        printf("Shell process closed.\n");
                    }
                    else
                    {
                        perror("Read error from shell PTY");
                    }
                    break;
                }

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

        // Close the master PTY and client socket
        close(master_fd);
        close(client_fd);
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
