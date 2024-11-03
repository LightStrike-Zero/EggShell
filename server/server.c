/**
 * @file server.c
 * @brief Server implementation for custom Telnet-like shell
 *
 * This server listens on a specified port, accepts incoming client connections,
 * allocates PTYs, spawns shell processes, and relays data between clients and shells.
 */

#include "server.h"

#include <pty.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <sys/wait.h>
/* these are used for logs */
#include <errno.h>
#include <stdarg.h>
#include <sys/file.h>

#define USER_FILE "users.txt"
#define MAX_USERS 100
#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 50

typedef struct {
    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
} User;

User users[MAX_USERS];
int user_count = 0;

/**
 * @brief Entry point for the server application.
 */
int main(int argc, char *argv[]) {
    int server_fd, port = DEFAULT_PORT;

    if (argc == 2) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port number: %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }
    }

    setup_signal_handlers();

    /* bind the server socket */
    setup_server(&server_fd, port);
    log_event("Server listening on port %d.\n", port);

    /* accept and handle incoming connections */
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t sin_size = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &sin_size);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        log_event("Received connection from %s:%d.\n",
          inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        /* child process handle the client */
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(client_fd);
            continue;
        }

        if (pid == 0) {
            /* Child Process */
            close(server_fd);
            handle_client(client_fd);
            close(client_fd);
            exit(EXIT_SUCCESS);
        } else {
            /* Parent Process */
            close(client_fd);
        }
    }

    close(server_fd);
    return 0;
}

/**
 * @brief Sets up the server socket.
 *
 * @param server_fd Pointer to store the server socket file descriptor.
 * @param port Port number to bind the server to.
 */
void setup_server(int *server_fd, const int port) {
    struct sockaddr_in server_addr;
    const int yes = 1;

    /* Create a TCP socket */
    if ((*server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* Avoid "Address already in use" error */
    if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        close(*server_fd);
        exit(EXIT_FAILURE);
    }

    /* bind the socket to the port */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    memset(&(server_addr.sin_zero), '\0', 8);

    if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        close(*server_fd);
        exit(EXIT_FAILURE);
    }

    /* listening on the socket */
    if (listen(*server_fd, BACKLOG) == -1) {
        perror("listen");
        close(*server_fd);
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Handles an individual client connection.
 *
 * @param client_fd The connected client socket file descriptor.
 */
void handle_client(const int client_fd) {

    if (user_count == 0 && !load_users()) {
        log_event("Failed to load user credentials.\n");
        close(client_fd);
        return;
    }

    char username[MAX_USERNAME_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int nbytes;

    // Prompt for username and receive input
    write(client_fd, "Username: ", 10);
    nbytes = read(client_fd, username, MAX_USERNAME_LENGTH - 1);
    if (nbytes <= 0) {
        send_response(client_fd, DISCONNECTION_NOTICE, "Disconnected during username input.");
        close(client_fd);
        return;
    }
    username[nbytes - 1] = '\0';

    // Prompt for password and receive input
    write(client_fd, "Password: ", 10);
    nbytes = read(client_fd, password, MAX_PASSWORD_LENGTH - 1);
    if (nbytes <= 0) {
        send_response(client_fd, DISCONNECTION_NOTICE, "Disconnected during password input.");
        close(client_fd);
        return;
    }
    password[nbytes - 1] = '\0';

    // Authenticate user
    if (!authenticate_user(username, password)) {
        send_response(client_fd, AUTHENTICATION_FAILED, NULL);
        log_event("Failed login attempt for user: %s\n", username);
        close(client_fd);
        return;
    }

    send_response(client_fd, AUTHENTICATION_SUCCESS, NULL);
    log_event("User %s authenticated successfully.\n", username);

    int master_fd, slave_fd;
    char slave_name[100];
    struct termios termp;
    struct winsize winp;

    tcgetattr(STDIN_FILENO, &termp);
    ioctl(STDIN_FILENO, TIOCGWINSZ, &winp);

    if (openpty(&master_fd, &slave_fd, slave_name, &termp, &winp) == -1) {
        perror("openpty");
        close(client_fd);
        return;
    }

    /* create the shell */
    pid_t shell_pid = fork();
    if (shell_pid < 0) {
        perror("fork");
        log_event("Failed to fork process for client_fd %d: %s\n", client_fd, strerror(errno));
        close(master_fd);
        close(slave_fd);
        close(client_fd);
        return;
    }

    if (shell_pid == 0) {
        /* execute Shell */
        log_event("Forked grandchild process (PID %d) to execute shell for client_fd %d.\n", getpid(), client_fd);
        close(master_fd);

        /* slave PTY as the controlling terminal */
        if (setsid() == -1) {
            perror("setsid");
            log_event("setsid failed in shell process (PID %d): %s\n", getpid(), strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (ioctl(slave_fd, TIOCSCTTY, NULL) == -1) {
            perror("ioctl TIOCSCTTY");
            log_event("ioctl TIOCSCTTY failed in shell process (PID %d): %s\n", getpid(), strerror(errno));
            exit(EXIT_FAILURE);
        }

        /* slave_fd to standard streams */
        if (dup2(slave_fd, STDIN_FILENO) == -1) {
            perror("dup2 STDIN");
            log_event("dup2 STDIN failed in shell process (PID %d): %s\n", getpid(), strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (dup2(slave_fd, STDOUT_FILENO) == -1) {
            perror("dup2 STDOUT");
            log_event("dup2 STDOUT failed in shell process (PID %d): %s\n", getpid(), strerror(errno));
            exit(EXIT_FAILURE);
        }
        if (dup2(slave_fd, STDERR_FILENO) == -1) {
            perror("dup2 STDERR");
            log_event("dup2 STDERR failed in shell process (PID %d): %s\n", getpid(), strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (slave_fd > STDERR_FILENO) {
            close(slave_fd);
        }

        /* set the TERM environment variable */
        setenv("TERM", "xterm-256color", 1);
        log_event("Set TERM environment variable to xterm-256color in shell process (PID %d).\n", getpid());

        /* Execute the egg_shell */
        log_event("Executing custom shell: ../shell/egg_shell in shell process (PID %d).\n", getpid());
        execl("../shell/egg_shell", "egg_shell", NULL);

        perror("execl");
        log_event("execl failed in shell process (PID %d): %s\n", getpid(), strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* parent process */
    close(slave_fd);

    /* transmit data between master PTY and client */
    relay_data(master_fd, client_fd);

    // Cleanup
    close(master_fd);
    kill(shell_pid, SIGKILL);
    waitpid(shell_pid, NULL, 0);
    send_response(client_fd, DISCONNECTION_NOTICE, "Session ended.");
    close(client_fd);
}

/**
 * @brief Relays data between the PTY master and the client socket.
 *
 * @param master_fd The PTY master file descriptor.
 * @param client_fd The client socket file descriptor.
 */
void relay_data(const int master_fd, const int client_fd) {
    fd_set read_fds;
    const int max_fd = (master_fd > client_fd) ? master_fd : client_fd;
    char buffer[BUFFER_SIZE];
    int nbytes;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(master_fd, &read_fds);
        FD_SET(client_fd, &read_fds);

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            log_event("select() failed: %s\n", strerror(errno));
            break;
        }

        // data from server to client
        if (FD_ISSET(master_fd, &read_fds)) {
            nbytes = read(master_fd, buffer, sizeof(buffer));
            if (nbytes < 0) {
                perror("read from master_fd");
                log_event("Failed to read from master_fd %d: %s\n", master_fd, strerror(errno));
                break;
            }
            if (nbytes == 0) {
                log_event("master_fd %d closed the connection.\n", master_fd);
                break;
            }
            if (write(client_fd, buffer, nbytes) != nbytes) {
                perror("write to client_fd");
                log_event("Failed to write to client_fd %d: %s\n", client_fd, strerror(errno));
                break;
            }
            buffer[nbytes] = '\0';
            log_event("Sent to client_fd %d: %s\n", client_fd, buffer);
        }

        // Data from client to server
        if (FD_ISSET(client_fd, &read_fds)) {
            nbytes = read(client_fd, buffer, sizeof(buffer));
            if (nbytes < 0) {
                perror("read from client_fd");
                log_event("Failed to read from client_fd %d: %s\n", client_fd, strerror(errno));
                break;
            }
            if (nbytes == 0) {
                log_event("client_fd %d closed the connection.\n", client_fd);
                break;
            }
            if (write(master_fd, buffer, nbytes) != nbytes) {
                perror("write to master_fd");
                log_event("Failed to write to master_fd %d: %s\n", master_fd, strerror(errno));
                break;
            }
            buffer[nbytes] = '\0';
            log_event("Received from client_fd %d: %s\n", client_fd, buffer);
        }
    }
}

/**
 * @brief Signal handler to reap zombie child processes.
 *
 * @param sig The signal number.
 */
void reap_zombie_processes(const int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/**
 * @brief Sets up signal handlers for the server.
 */
void setup_signal_handlers() {
    struct sigaction sa;

    /* zombie child process cleanup */
    sa.sa_handler = reap_zombie_processes;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction SIGCHLD");
        exit(EXIT_FAILURE);
    }

    /* ignoring SIGPIPE to stop server from dying when accidentally writing to a closed socket */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("sigaction SIGPIPE");
        exit(EXIT_FAILURE);
    }
}

/**
 * @brief Logs events to both stdout and server.log with timestamps.
 *
 * @param format The format string (printf-style).
 * @param ... accepts any number of arguments to be formatted, stored in va_list args.
 */
void log_event(const char *format, ...) {
    va_list args;
    va_start(args, format);

    // current time
    const time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    if (tm_info == NULL) {
        perror("localtime");
        va_end(args);
        return;
    }

    char time_buffer[26];
    if (strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info) == 0) {
        fprintf(stderr, "strftime returned 0");
        va_end(args);
        return;
    }

    char log_message[1024];
    snprintf(log_message, sizeof(log_message), "[%s] ", time_buffer);
    vsnprintf(log_message + strlen(log_message), sizeof(log_message) - strlen(log_message), format, args);

    va_end(args);

    printf("%s", log_message);
    fflush(stdout);

    const int log_fd = open("server.log", O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (log_fd == -1) {
        perror("open log file");
        return;
    }

    if (flock(log_fd, LOCK_EX) == -1) {
        perror("flock");
        close(log_fd);
        return;
    }

    const ssize_t bytes_written = write(log_fd, log_message, strlen(log_message));
    if (bytes_written == -1) {
        perror("write to log file");
    }

    if (flock(log_fd, LOCK_UN) == -1) {
        perror("flock unlock");
    }
    close(log_fd);
}

// Load users from file
int load_users() {
    FILE *file = fopen(USER_FILE, "r");
    if (!file) {
        perror("Could not open user file");
        return 0;
    }

    while (fscanf(file, "%49[^:]:%49s\n", users[user_count].username, users[user_count].password) == 2) {
        user_count++;
        if (user_count >= MAX_USERS) {
            fprintf(stderr, "Max user limit reached in user file.\n");
            break;
        }
    }
    fclose(file);
    return 1;
}

// Authenticate user
int authenticate_user(const char *username, const char *password) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 &&
            strcmp(users[i].password, password) == 0) {
            return 1;
            }
    }
    return 0;
}

void send_response(int client_fd, ServerResponse response_code, const char *message) {
    char buffer[BUFFER_SIZE];

    // Map response code to a message if no custom message is provided
    switch (response_code) {
    case CONNECTION_SUCCESS:
        snprintf(buffer, sizeof(buffer), LIGHT_GREEN"Connection successful.\n"RESET);
        break;
    case CONNECTION_FAILURE:
        snprintf(buffer, sizeof(buffer), RED"Connection failed.\n"RESET);
        break;
    case AUTHENTICATION_FAILED:
        snprintf(buffer, sizeof(buffer), RED"Authentication failed.\n"RESET);
        break;
    case AUTHENTICATION_SUCCESS:
        snprintf(buffer, sizeof(buffer), LIGHT_GREEN"Authentication successful.\n"RESET);
        break;
    case COMMAND_EXECUTION_ERROR:
        snprintf(buffer, sizeof(buffer), RED"Error executing command.\n"RESET);
        break;
    case DISCONNECTION_NOTICE:
        snprintf(buffer, sizeof(buffer), RED"Client disconnected.\n"RESET);
        break;
    default:
        snprintf(buffer, sizeof(buffer), RED"Unknown response code.\n"RESET);
        break;
    }

    // If a custom message is provided, append it
    if (message != NULL) {
        strncat(buffer, message, sizeof(buffer) - strlen(buffer) - 1);
    }

    // Send the response to the client
    write(client_fd, buffer, strlen(buffer));

    // Log the response
    log_event("Sent to client_fd %d: %s", client_fd, buffer);
}