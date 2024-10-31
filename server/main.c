#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

int setup_server_socket(int port);
int authenticate(int client_fd);
void handle_client(int client_fd);

int main()
{
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
        else if (client_count < 100)
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
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

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

    printf("Server listening on port %d\n", PORT);

    // Keep the server running indefinitely
    while (1)
    {
        // pause(); // Wait indefinitely
    }

    close(sockfd);
    return 0;
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

    // Parse username and password from buffer
    char received_username[USERNAME_LENGTH], received_password[PASSWORD_LENGTH];
    sscanf(buffer, "%s %s", received_username, received_password);

    // Check against the user table
    if (strcmp(received_username, userTable.username) == 0 &&
        strcmp(received_password, userTable.password) == 0)
    {
        write(client_fd, "Authentication successful\n", strlen("Authentication successful\n"));
        return 1; // Authentication success
    }
    else
    {
        write(client_fd, "Authentication failed\n", strlen("Authentication failed\n"));
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

    char command_buffer[256];
    while (1)
    {
        memset(command_buffer, 0, sizeof(command_buffer));
        int n = read(client_fd, command_buffer, sizeof(command_buffer) - 1);
        if (n <= 0)
            break; // Client disconnected

        command_buffer[n] = '\0'; // Null-terminate the command string

        // Check if client wants to exit
        if (strcmp(command_buffer, "Bye bye") == 0)
        {
            write(client_fd, "Disconnecting...", strlen("Disconnecting..."));
            break;
        }

        // Respond to other commands
        write(client_fd, "Command received", strlen("Command received"));
    }

    close(client_fd); // Close connection when done
    exit(0);          // End child process
}
