#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "mailman.h"

#define PORT 42010
#define MAX_COMMAND_LENGTH 255

void serverListen() {
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    // Packet size, corresponding to two integers and a command
    size_t packet_size = sizeof(int) * 2 + MAX_COMMAND_LENGTH;
    char buffer[packet_size];

    // Open socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error opening socket");
        exit(1);
    }

    // Configure server address
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("Error on binding");
        close(sockfd);
        exit(1);
    }

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    printf("Server listening on port %d\n", PORT);

    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0) {
            perror("Error on accept");
            continue;
        }

        // Read data into buffer
        int n = read(newsockfd, buffer, packet_size);
        if (n <= 0) {
            perror("Error reading packet from client");
            close(newsockfd);
            continue;
        }

        // Parse integers and command from the buffer
        int *int_ptr = (int *)buffer;
        int value1 = int_ptr[0];
        int value2 = int_ptr[1];
        char *command = buffer + sizeof(int) * 2;

        // Log to file
        FILE *file = fopen("output.txt", "a");
        if (!file) {
            perror("Error opening output file");
            close(newsockfd);
            continue;
        }

        fprintf(file, "Received values: %d, %d\n", value1, value2);
        fprintf(file, "Received command: %s\n", command);
        fclose(file);

        // Respond to client
        if (strcmp(command, "Bye bye") == 0) {
            write(newsockfd, "Disconnecting...", 16);
            close(newsockfd);
            break;
        } else {
            write(newsockfd, "Command received", 16);
        }

        close(newsockfd);
    }

    close(sockfd);
}

int main() {
    serverListen();
    return 0;
}