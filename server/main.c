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

#define BUFFER_SIZE 1024

#define USERNAME_LENGTH 30
#define PASSWORD_LENGTH 12
#define PORT 40210

float version = 0.27;

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

// Function prototypes
int setup_server_socket(int port);
int authenticate(int client_fd);
void handle_client(int client_fd);
ssize_t strip_cr(char *buffer, ssize_t nbytes);
void sigchld_handler(int signum);

int main()
{
    printf("Version: %.2f\n", version);

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

        printf("Connection from %s:%d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

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

    // Allow address reuse
    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(sockfd);
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
    return j;
}

int authenticate(int client_fd)
{
    char buffer[USERNAME_LENGTH + PASSWORD_LENGTH + 2]; // +1 for space, +1 for '\0'
    int n = read(client_fd, buffer, sizeof(buffer) - 1); // Read into buffer
    if (n <= 0)
    {
        perror("Error reading from client during authentication");
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
    if (sscanf(buffer, "%29s %11s", received_username, received_password) != 2)
    {
        write(client_fd, "Invalid authentication format. Use: <username> <password>\n", strlen("Invalid authentication format. Use: <username> <password>\n"));
        return 0;
    }

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

    // Allocate a PTY and fork the shell
    int master_fd;
    pid_t pid = forkpty(&master_fd, NULL, NULL, NULL);
    if (pid < 0)
    {
        perror("forkpty failed");
        close(client_fd);
        exit(1);
    }

    if (pid == 0)
    {
        // Child process: execute the shell
        // Replace with your custom shell if needed
        execlp("/bin/bash", "/bin/bash", "--noprofile", "--norc", NULL);
        perror("execlp failed");
        exit(1);
    }

    // Parent process: communicate between client_fd and master_fd
    fd_set read_fds;
    int max_fd = (client_fd > master_fd) ? client_fd : master_fd;
    char buffer[BUFFER_SIZE];
    ssize_t nbytes;

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
                // Optional: Handle special commands like "exit" or "quit"
                // For simplicity, directly forward to shell
                if (write(master_fd, buffer, nbytes) != nbytes)
                {
                    perror("write to master_fd failed");
                    break;
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
                if (write(client_fd, buffer, nbytes) != nbytes)
                {
                    perror("write to client_fd failed");
                    break;
                }
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
    // Optionally, terminate the shell process
    kill(pid, SIGKILL);
    waitpid(pid, NULL, 0);
}

void sigchld_handler(int signum)
{
    // Save and restore errno to avoid side effects
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
    errno = saved_errno;
}
