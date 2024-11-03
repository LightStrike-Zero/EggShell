//
// Created by Shaun on 3/11/2024.
//

#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stddef.h>
#include <stdint.h>


typedef enum {
    AUTH_REQUEST,        // Authentication request (username/password)
    AUTH_RESPONSE,       // Authentication response (success/failure)
    COMMAND,             // Command sent from client to server
    RESPONSE,            // Response from server to client (command output)
    DISCONNECT           // Disconnection notice
} MessageType;

typedef struct {
    MessageType type;    // Type of message
    uint16_t length;     // Length of message content
    char content[512];   // Message content
} Message;

int encode_message(const Message *msg, char *buffer, size_t buffer_size);
int decode_message(const char *buffer, size_t buffer_size, Message *msg);

#endif //PROTOCOL_H
