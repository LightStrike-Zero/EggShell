//
// Created by Shaun on 3/11/2024.
//
#include "protocol.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>  // For hton/ntoh functions


// Send a message over a socket
int send_message(int client_fd, const Message *msg) {
    // Calculate the total size of the message
    size_t total_size = sizeof(ResponseCode) + sizeof(uint16_t) + msg->content_length;
    const char *data_ptr = (const char *)msg;
    size_t bytes_sent = 0;

    while (bytes_sent < total_size) {
        ssize_t result = write(client_fd, data_ptr + bytes_sent, total_size - bytes_sent);
        if (result < 0) {
            perror("write to client_fd");
            return -1;  // Error occurred
        }
        bytes_sent += result;
    }
    return bytes_sent;
}

// Receive a message from a socket
int receive_message(int socket_fd, Message *msg) {
    // Read the fixed-size part first (ResponseCode + content_length)
    int header_size = sizeof(ResponseCode) + sizeof(uint16_t);
    int nbytes = read(socket_fd, msg, header_size);
    if (nbytes <= 0) return -1;  // Connection closed or error

    // Read the actual content if content_length is greater than 0
    if (msg->content_length > 0 && msg->content_length <= sizeof(msg->content)) {
        int content_bytes = read(socket_fd, msg->content, msg->content_length);
        if (content_bytes <= 0) return -1;  // Connection closed or error
        msg->content[content_bytes] = '\0'; // Null-terminate the content
    } else {
        msg->content[0] = '\0';  // Ensure content is null-terminated if no content
    }

    return nbytes + msg->content_length;  // Total bytes read
}