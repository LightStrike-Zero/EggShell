/**
* @file protocol.c
 * @brief Implementation of the protocol
 *
 *
 * @author Shaun Matthews & Louise Barjaktarevic
 * @date 30/10/24
 */

#include "protocol.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>  // For hton/ntoh functions


int send_message(int client_fd, const Message *msg) {
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

int receive_message(int socket_fd, Message *msg) {
    int header_size = sizeof(ResponseCode) + sizeof(uint16_t);
    int nbytes = read(socket_fd, msg, header_size);
    if (nbytes <= 0) return -1;  // Connection closed or error

    if (msg->content_length > 0 && msg->content_length <= sizeof(msg->content)) {
        int content_bytes = read(socket_fd, msg->content, msg->content_length);
        if (content_bytes <= 0) return -1;
        msg->content[content_bytes] = '\0';
    } else {
        msg->content[0] = '\0';
    }

    return nbytes + msg->content_length;  // Total bytes read
}