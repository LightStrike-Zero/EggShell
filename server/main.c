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
#include <fcntl.h>

#define BUFFER_SIZE 1024
#define USERNAME_LENGTH 30
#define PASSWORD_LENGTH 12
#define PORT 40210
#define CUSTOM_SHELL_PATH "../shell/egg_shell"

#define DEBUG 1

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

    char received_username[USERNAME_LENGTH], received_password[PASSWORD_LENGTH];
    sscanf(buffer, "%29s %11s", received_username, received_password);

    if (strcmp(received_username, userTable.username) == 0 &&
        strcmp(received_password, userTable.password) == 0) {
        write(client_fd, "Authentication successful\n", strlen("Authentication successful\n"));
        return 1;
    } else {
        write(client_fd, "Authentication failed\n", strlen("Authentication failed\n"));
        return 0;
    }
}

void handle_client(int client_fd) {
    int shell_to_server[2];  // Pipe from shell stdout to server
    int server_to_shell[2];  // Pipe from server to shell stdin

    if (pipe(shell_to_server) == -1 || pipe(server_to_shell) == -1) {
        perror("pipe failed");
        close(client_fd);
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {
        dup2(server_to_shell[0], STDIN_FILENO);  // Read from server to shell pipe as stdin
        dup2(shell_to_server[1], STDOUT_FILENO); // Write to shell to server pipe as stdout
        dup2(shell_to_server[1], STDERR_FILENO); // Redirect stderr to the same pipe for consistency

        close(shell_to_server[0]);
        close(server_to_shell[1]);

        execl(CUSTOM_SHELL_PATH, CUSTOM_SHELL_PATH, NULL);
        perror("execl failed");
        exit(1);
    } else if (pid > 0) {
        close(shell_to_server[1]);
        close(server_to_shell[0]);

        char buffer[BUFFER_SIZE];
        ssize_t bytes_read;
        char output_buffer[BUFFER_SIZE * 4];  // Accumulate output for each command
        int output_len = 0;

        while (1) {
            bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
            if (bytes_read <= 0) {
                DEBUG_PRINT("Client disconnected or read error for client fd: %d\n", client_fd);
                break;
            }
            buffer[bytes_read] = '\0';

            write(server_to_shell[1], buffer, bytes_read);
            write(server_to_shell[1], "\n", 1);

            output_len = 0;
            while ((bytes_read = read(shell_to_server[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytes_read] = '\0';
                strncpy(output_buffer + output_len, buffer, sizeof(output_buffer) - output_len - 1);
                output_len += bytes_read;
                output_buffer[output_len] = '\0';

                if (strstr(output_buffer, "%") != NULL) {
                    DEBUG_PRINT("End of command output detected for client fd %d\n", client_fd);
                    break;
                }
            }

            write(client_fd, output_buffer, output_len);

            memset(output_buffer, 0, sizeof(output_buffer));
        }

        close(client_fd);
        close(shell_to_server[0]);
        close(server_to_shell[1]);
        waitpid(pid, NULL, 0);
        DEBUG_PRINT("Closed connection and cleaned up for client fd: %d\n", client_fd);
    } else {
        perror("fork failed");
        close(client_fd);
        close(shell_to_server[0]);
        close(shell_to_server[1]);
        close(server_to_shell[0]);
        close(server_to_shell[1]);
    }
}

void sigchld_handler(int signum) {
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}
