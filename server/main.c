/**
 * @file server.c
 * @brief Server implementation for custom Telnet-like shell
 *
 * This server listens on a specified port, accepts incoming client connections,
 * allocates PTYs, spawns shell processes, and relays data between clients and shells.
 */

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
#include <sys/wait.h>

#define BUFFER_SIZE 4096
/* Constants */
#define DEFAULT_PORT 40210
#define BACKLOG 10          // Number of pending connections queue will hold
#define BUFFER_SIZE 4096    // Buffer size for data relay

/* Function Declarations */
void setup_server(int *server_fd, int port);
void handle_client(int client_fd);
void relay_data(int master_fd, int client_fd);
void reap_zombie_processes(int signo);
void setup_signal_handlers();

/**
 * @brief Entry point for the server application.
 */
int main(int argc, char *argv[]) {
    int server_fd, port = DEFAULT_PORT;

    /* Optionally, allow port to be specified via command-line arguments */
    if (argc == 2) {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            fprintf(stderr, "Invalid port number: %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }
    }

    /* Setup signal handlers */
    setup_signal_handlers();

    /* Initialize and bind the server socket */
    setup_server(&server_fd, port);
    printf("Server listening on port %d\n", port);

    /* Main loop: accept and handle incoming connections */
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t sin_size = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &sin_size);
        if (client_fd == -1) {
            perror("accept");
            continue;
        }

        printf("Received connection from %s:%d\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        /* Fork a child process to handle the client */
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            close(client_fd);
            continue;
        }

        if (pid == 0) {
            /* Child Process */
            close(server_fd); // Child doesn't need the listening socket
            handle_client(client_fd);
            close(client_fd);
            exit(EXIT_SUCCESS);
        } else {
            /* Parent Process */
            close(client_fd); // Parent doesn't need the connected socket
        }
    }

    /* Close the server socket (unreachable in this example) */
    close(server_fd);
    return 0;
}

/**
 * @brief Sets up the server socket.
 *
 * @param server_fd Pointer to store the server socket file descriptor.
 * @param port Port number to bind the server to.
 */
void setup_server(int *server_fd, int port) {
    struct sockaddr_in server_addr;
    int yes = 1;

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

    /* Bind the socket to the specified port */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    server_addr.sin_port = htons(port);
    memset(&(server_addr.sin_zero), '\0', 8); // Zero the rest of the struct

    if (bind(*server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        close(*server_fd);
        exit(EXIT_FAILURE);
    }

    /* Start listening on the socket */
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
void handle_client(int client_fd) {
    int master_fd, slave_fd;
    pid_t shell_pid;
    char slave_name[100];
    struct termios termp;
    struct winsize winp;

    /* Initialize termios and winsize if needed */
    tcgetattr(STDIN_FILENO, &termp);
    ioctl(STDIN_FILENO, TIOCGWINSZ, &winp);

    /* Open PTY */
    if (openpty(&master_fd, &slave_fd, slave_name, &termp, &winp) == -1) {
        perror("openpty");
        close(client_fd);
        return;
    }

    /* Fork to create the shell process */
    shell_pid = fork();
    if (shell_pid < 0) {
        perror("fork");
        close(master_fd);
        close(slave_fd);
        close(client_fd);
        return;
    }

    if (shell_pid == 0) {
        /* Grandchild Process: Execute Shell */
        close(master_fd); // Close master in child

        /* Set the slave PTY as the controlling terminal */
        if (setsid() == -1) {
            perror("setsid");
            exit(EXIT_FAILURE);
        }

        if (ioctl(slave_fd, TIOCSCTTY, NULL) == -1) {
            perror("ioctl TIOCSCTTY");
            exit(EXIT_FAILURE);
        }

        /* Duplicate slave_fd to standard streams */
        dup2(slave_fd, STDIN_FILENO);
        dup2(slave_fd, STDOUT_FILENO);
        dup2(slave_fd, STDERR_FILENO);

        if (slave_fd > STDERR_FILENO) {
            close(slave_fd);
        }

        /* Optionally, set the TERM environment variable */
        setenv("TERM", "xterm-256color", 1);

        /* Execute the custom shell */
        execl("../shell/egg_shell", "egg_shell", NULL);

        /* If exec fails */
        perror("execl");
        exit(EXIT_FAILURE);
    }

    /* Parent Process */
    close(slave_fd); // Close slave in parent

    /* Relay data between master PTY and client socket */
    relay_data(master_fd, client_fd);

    /* Cleanup */
    close(master_fd);
    kill(shell_pid, SIGKILL);
    waitpid(shell_pid, NULL, 0);
    close(client_fd);
}

/**
 * @brief Relays data between the PTY master and the client socket.
 *
 * @param master_fd The PTY master file descriptor.
 * @param client_fd The client socket file descriptor.
 */
void relay_data(int master_fd, int client_fd) {
    fd_set read_fds;
    int max_fd = (master_fd > client_fd) ? master_fd : client_fd;
    char buffer[BUFFER_SIZE];
    int nbytes;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(master_fd, &read_fds);
        FD_SET(client_fd, &read_fds);

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            break;
        }

        // Data from PTY master to client
        if (FD_ISSET(master_fd, &read_fds)) {
            nbytes = read(master_fd, buffer, sizeof(buffer));
            if (nbytes <= 0) {
                // EOF or error
                break;
            }
            if (write(client_fd, buffer, nbytes) != nbytes) {
                perror("write to client");
                break;
            }
        }

        // Data from client to PTY master
        if (FD_ISSET(client_fd, &read_fds)) {
            nbytes = read(client_fd, buffer, sizeof(buffer));
            if (nbytes <= 0) {
                // EOF or error
                break;
            }
            if (write(master_fd, buffer, nbytes) != nbytes) {
                perror("write to master pty");
                break;
            }
        }
    }
}

/**
 * @brief Signal handler to reap zombie child processes.
 *
 * @param signo The signal number.
 */
void reap_zombie_processes(int signo) {
    (void)signo; // Unused parameter
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/**
 * @brief Sets up signal handlers for the server.
 */
void setup_signal_handlers() {
    struct sigaction sa;

    /* Reap zombie child processes */
    sa.sa_handler = reap_zombie_processes;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction SIGCHLD");
        exit(EXIT_FAILURE);
    }

    /* Ignore SIGPIPE to prevent server from crashing when writing to a closed socket */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("sigaction SIGPIPE");
        exit(EXIT_FAILURE);
    }
}

