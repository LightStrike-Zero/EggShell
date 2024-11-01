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

#define BUFFER_SIZE 1024
#define USERNAME_LENGTH 30
#define PASSWORD_LENGTH 12
#define PORT 40210
#define COMMAND_COMPLETION_MARKER "__COMMAND_COMPLETED__"

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

// Function prototypes
int setup_server_socket(int port);
int authenticate(int client_fd);
void handle_client(int client_fd);
ssize_t strip_cr(char *buffer, ssize_t nbytes);
void sigchld_handler(int signum);
int contains_marker(const char *buffer, size_t len);

int main() {
    // Set up the SIGCHLD handler to reap zombie processes
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    // Step 1: Initialize server socket
    int server_fd = setup_server_socket(PORT);

    // Main server loop
    while (1) {
        struct sockaddr_in cli_addr;
        socklen_t clilen = sizeof(cli_addr);

        // Step 2: Accept a new client connection
        int client_fd = accept(server_fd, (struct sockaddr *)&cli_addr, &clilen);
        if (client_fd < 0) {
            perror("Error on accept");
            continue;
        }

        // Step 3: Fork a new process for each client
        pid_t pid = fork();
        if (pid == 0) { // Child process
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

    // Open socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        exit(1);
    }

    // Configure server address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    // Bind socket to port
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error on binding");
        close(sockfd);
        exit(1);
    }

    // Start listening on the socket
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
    if (!authenticate(client_fd)) {
        close(client_fd);
        exit(0);
    }

    int shell_to_server[2];
    int server_to_shell[2];

    if (pipe(shell_to_server) == -1 || pipe(server_to_shell) == -1) {
        perror("Pipe creation failed");
        close(client_fd);
        exit(1);
    }

    pid_t pid = fork();
    if (pid == 0) { // Child process (shell)
        if (dup2(server_to_shell[0], STDIN_FILENO) == -1 ||
            dup2(shell_to_server[1], STDOUT_FILENO) == -1 ||
            dup2(shell_to_server[1], STDERR_FILENO) == -1) {
            perror("dup2 failed");
            exit(1);
        }

        close(server_to_shell[1]);
        close(shell_to_server[0]);
        close(client_fd);

        const char *shell_path = "../shell/egg_shell";
        printf("Attempting to execute shell at path: %s\n", shell_path);

        execlp(shell_path, "egg_shell", NULL);
        
        // If execlp returns, it means there was an error
        perror("Failed to execute custom shell");
        exit(1);
    } else if (pid > 0) { // Parent process (server handling client)
        close(server_to_shell[0]);
        close(shell_to_server[1]);

        fd_set read_fds;
        int max_fd = (client_fd > shell_to_server[0]) ? client_fd : shell_to_server[0];
        char buffer[BUFFER_SIZE];
        ssize_t nbytes;

        while (1) {
            nbytes = read(client_fd, buffer, sizeof(buffer) - 1);
            if (nbytes <= 0) {
                if (nbytes < 0) {
                    perror("Error reading from client");
                } else {
                    printf("Client disconnected.\n");
                }
                break;
            }
            buffer[nbytes] = '\0';

            ssize_t clean_nbytes = strip_cr(buffer, nbytes);
            buffer[clean_nbytes] = '\0';

            printf("Command received from client: \"%s\"\n", buffer);

            if (strcmp(buffer, "exit") == 0 || strcmp(buffer, "quit") == 0) {
                write(client_fd, "Disconnecting...\n", strlen("Disconnecting...\n"));
                printf("Received termination command from client.\n");
                break;
            }

            const char *marker = "; echo __COMMAND_COMPLETED__\n";
            char command_with_marker[BUFFER_SIZE + 50];
            snprintf(command_with_marker, sizeof(command_with_marker), "%s%s", buffer, marker);

            ssize_t total_written = 0;
            ssize_t command_len = strlen(command_with_marker);
            while (total_written < command_len) {
                ssize_t bytes_written = write(server_to_shell[1], command_with_marker + total_written, command_len - total_written);
                if (bytes_written <= 0) {
                    perror("Write error to shell");
                    break;
                }
                total_written += bytes_written;
            }

            char shell_buffer[BUFFER_SIZE];
            size_t shell_buffer_len = 0;
            int command_completed = 0;

            while (!command_completed) {
                FD_ZERO(&read_fds);
                FD_SET(shell_to_server[0], &read_fds);

                int activity = select(shell_to_server[0] + 1, &read_fds, NULL, NULL, NULL);
                if (activity < 0 && errno != EINTR) {
                    perror("Select error");
                    break;
                }

                if (FD_ISSET(shell_to_server[0], &read_fds)) {
                    nbytes = read(shell_to_server[0], shell_buffer + shell_buffer_len, sizeof(shell_buffer) - shell_buffer_len - 1);
                    if (nbytes <= 0) {
                        if (nbytes < 0) {
                            perror("Error reading from shell");
                        } else {
                            printf("Shell process terminated.\n");
                        }
                        break;
                    }

                    shell_buffer_len += nbytes;
                    shell_buffer[shell_buffer_len] = '\0';

                    ssize_t total_written = 0;
                    size_t data_to_write = shell_buffer_len;

                    char *marker_position = strstr(shell_buffer, COMMAND_COMPLETION_MARKER);
                    if (marker_position != NULL) {
                        command_completed = 1;
                        data_to_write = marker_position - shell_buffer;
                    }

                    while (total_written < data_to_write) {
                        ssize_t bytes_written = write(client_fd, shell_buffer + total_written, data_to_write - total_written);
                        if (bytes_written <= 0) {
                            perror("Write error to client");
                            break;
                        }
                        total_written += bytes_written;
                    }

                    if (command_completed && (shell_buffer_len > data_to_write + strlen(COMMAND_COMPLETION_MARKER))) {
                        size_t remaining_data = shell_buffer_len - (data_to_write + strlen(COMMAND_COMPLETION_MARKER));
                        memmove(shell_buffer, shell_buffer + data_to_write + strlen(COMMAND_COMPLETION_MARKER), remaining_data);
                        shell_buffer_len = remaining_data;
                    } else {
                        shell_buffer_len = 0;
                    }
                }
            }
        }

        close(server_to_shell[1]);
        close(shell_to_server[0]);
        close(client_fd);
    } else {
        perror("Fork failed");
        close(client_fd);
        exit(1);
    }
}

void sigchld_handler(int signum) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

int contains_marker(const char *buffer, size_t len) {
    static char marker[] = COMMAND_COMPLETION_MARKER;
    size_t marker_len = strlen(marker);

    if (len < marker_len) {
        return 0;
    }

    for (size_t i = 0; i <= len - marker_len; i++) {
        if (strncmp(buffer + i, marker, marker_len) == 0) {
            return 1;
        }
    }
    return 0;
}
