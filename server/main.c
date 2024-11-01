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
#include <fcntl.h>    // Added for file descriptor flags

#define BUFFER_SIZE 1024
#define USERNAME_LENGTH 30
#define PASSWORD_LENGTH 12
#define PORT 40210
#define TIMEOUT_SEC 0
#define TIMEOUT_USEC 100000  // 100 ms timeout
#define CUSTOM_SHELL_PATH "../shell/egg_shell"

#define DEBUG 1  // Set to 1 to enable debug output, 0 to disable

#define DEBUG_PRINT(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, fmt, __VA_ARGS__); } while (0)

struct ClientInfo {
    int client_socket;
    pid_t pid;
};

struct UserTable {
    char username[USERNAME_LENGTH];
    char password[PASSWORD_LENGTH];
} userTable = {"test", "test"};

struct ClientInfo client_list[100];
int client_count = 0;

int setup_server_socket(int port);
int authenticate(int client_fd);
void handle_client(int client_fd);
ssize_t strip_cr(char *buffer, ssize_t nbytes);
void sigchld_handler(int signum);

int main() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    int server_fd = setup_server_socket(PORT);

    while (1) {
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);

        int client_fd = accept(server_fd, (struct sockaddr *)&cli_addr, &clilen);
        if (client_fd < 0) {
            perror("Error on accept");
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {
            close(server_fd);
            handle_client(client_fd);
            exit(0);
        } else if (pid > 0) {
            if (client_count < 100) {
                client_list[client_count].client_socket = client_fd;
                client_list[client_count].pid = pid;
                client_count++;
            } else {
                perror("Max client limit reached.");
                close(client_fd);
            }
            close(client_fd);
        } else {
            perror("Fork failed");
            close(client_fd);
        }
    }

    close(server_fd);
    return 0;
}

int setup_server_socket(int port) {
    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error on binding");
        close(sockfd);
        exit(1);
    }

    if (listen(sockfd, 5) < 0) {
        perror("Error on listen");
        close(sockfd);
        exit(1);
    }

    printf("Server listening on port %d\n", port);
    return sockfd;
}

ssize_t strip_cr(char *buffer, ssize_t nbytes) {
    ssize_t j = 0;
    for (ssize_t i = 0; i < nbytes; i++) {
        if (buffer[i] != '\r') {
            buffer[j++] = buffer[i];
        }
    }
    buffer[j] = '\0';
    printf("After strip_cr: \"%s\"\n", buffer);
    return j;
}

int authenticate(int client_fd) {
    char buffer[USERNAME_LENGTH + PASSWORD_LENGTH + 1];
    int n = read(client_fd, buffer, sizeof(buffer) - 1);
    if (n <= 0) {
        perror("Error reading from client");
        return 0;
    }

    buffer[n] = '\0';
    ssize_t clean_nbytes = strip_cr(buffer, n);
    buffer[clean_nbytes] = '\0';

    printf("Authentication input after strip_cr: \"%s\"\n", buffer);

    char received_username[USERNAME_LENGTH], received_password[PASSWORD_LENGTH];
    sscanf(buffer, "%29s %11s", received_username, received_password);

    if (strcmp(received_username, userTable.username) == 0 &&
        strcmp(received_password, userTable.password) == 0) {
        write(client_fd, "Authentication successful\n", strlen("Authentication successful\n"));
        printf("User \"%s\" authenticated successfully.\n", received_username);
        return 1;
    } else {
        write(client_fd, "Authentication failed\n", strlen("Authentication failed\n"));
        printf("User \"%s\" failed to authenticate.\n", received_username);
        return 0;
    }
}

void handle_client(int client_fd) {
    DEBUG_PRINT("Client connected with fd: %d\n", client_fd);

    if (!authenticate(client_fd)) {
        DEBUG_PRINT("Authentication failed for client fd: %d\n", client_fd);
        close(client_fd);
        return;
    }
    DEBUG_PRINT("Authentication successful for client fd: %d\n", client_fd);

    int in_pipe[2], out_pipe[2];
    if (pipe(in_pipe) == -1 || pipe(out_pipe) == -1) {
        perror("pipe");
        close(client_fd);
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Child process: execute egg_shell
        DEBUG_PRINT("Forked child process for client fd: %d\n", client_fd);

        // Redirect stdin and stdout to the pipes
        dup2(in_pipe[0], STDIN_FILENO);
        dup2(out_pipe[1], STDOUT_FILENO);
        dup2(out_pipe[1], STDERR_FILENO);

        // Close unused pipe ends in the child
        close(in_pipe[1]);
        close(out_pipe[0]);
        close(client_fd);

        DEBUG_PRINT("Executing egg_shell for client fd: %d\n", client_fd);
        execl(CUSTOM_SHELL_PATH, CUSTOM_SHELL_PATH, NULL);
        perror("execl failed");
        exit(1);
    } else if (pid > 0) {
        // Parent process: communicate with client and shell via pipes
        close(in_pipe[0]);
        close(out_pipe[1]);

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;
        fd_set read_fds;
        struct timeval timeout;

        while (1) {
            // Wait for command input from the client
            FD_ZERO(&read_fds);
            FD_SET(client_fd, &read_fds);

            int max_fd = (client_fd > out_pipe[0]) ? client_fd : out_pipe[0];
            int activity = select(max_fd + 1, &read_fds, NULL, NULL, NULL);

            if (activity < 0 && errno != EINTR) {
                perror("select");
                DEBUG_PRINT("Select failed for client fd: %d\n", client_fd);
                break;
            }

            // Check if there’s data from the client (command input)
            if (FD_ISSET(client_fd, &read_fds)) {
                bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
                if (bytes_read <= 0) {
                    DEBUG_PRINT("Client disconnected or read error for client fd: %d\n", client_fd);
                    break;  // Client disconnected
                }
                buffer[bytes_read] = '\0';
                DEBUG_PRINT("Received command from client fd %d: %s\n", client_fd, buffer);

                // Send command to egg_shell
                strncat(buffer, "\n", sizeof(buffer) - strlen(buffer) - 1);
                write(in_pipe[1], buffer, strlen(buffer));
                DEBUG_PRINT("Command sent to shell for client fd %d: %s\n", client_fd, buffer);

                // Start reading the output in chunks with a longer timeout
                int output_complete = 0;
                while (!output_complete) {
                    // Set a longer timeout for this loop
                    timeout.tv_sec = TIMEOUT_SEC;
                    timeout.tv_usec = TIMEOUT_USEC;

                    FD_ZERO(&read_fds);
                    FD_SET(out_pipe[0], &read_fds);

                    int ready = select(out_pipe[0] + 1, &read_fds, NULL, NULL, &timeout);

                    if (ready < 0) {
                        perror("select");
                        break;
                    } else if (ready == 0) {
                        // Timeout reached with no more data, assume output is complete
                        // DEBUG_PRINT("Extended timeout reached, assuming command output is complete\n");
                        output_complete = 1;
                        break;
                    }

                    // Read output from egg_shell
                    bytes_read = read(out_pipe[0], buffer, sizeof(buffer) - 1);
                    if (bytes_read <= 0) {
                        DEBUG_PRINT("Shell process closed output for client fd: %d\n", client_fd);
                        output_complete = 1;
                        break;  // Shell process finished
                    }
                    buffer[bytes_read] = '\0';
                    DEBUG_PRINT("Received chunk from shell for client fd %d: %s\n", client_fd, buffer);

                    // Send output chunk directly to the client
                    write(client_fd, buffer, bytes_read);
                    DEBUG_PRINT("Output chunk sent to client fd %d\n", client_fd);
                }
            }
        }

        // Clean up: Close pipes and client connection
        close(in_pipe[1]);
        close(out_pipe[0]);
        close(client_fd);
        waitpid(pid, NULL, 0);  // Wait for the child process to finish
        DEBUG_PRINT("Closed connection and cleaned up for client fd: %d\n", client_fd);
    } else {
        perror("fork");
        close(in_pipe[0]);
        close(in_pipe[1]);
        close(out_pipe[0]);
        close(out_pipe[1]);
        close(client_fd);
    }
}
void sigchld_handler(int signum) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}
