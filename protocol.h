//
// Created by Shaun on 3/11/2024.
//

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    RESPONSE_OK,                // General success
    RESPONSE_FAIL,              // General failure
    AUTH_SUCCESS,               // Authentication successful
    AUTH_FAIL,                  // Authentication failed
    CONNECTION_SUCCESS,         // Connection established successfully
    CONNECTION_FAILURE,         // Connection failed
    COMMAND_SUCCESS,            // Command executed successfully
    COMMAND_FAIL,               // Command execution failed
    MESSAGE_TOO_LONG,           // Message content too long
} ResponseCode;

typedef enum {
    ESCAPE_CODE_NONE,
    ESCAPE_CODE_UP_ARROW,
    ESCAPE_CODE_DOWN_ARROW,
    ESCAPE_CODE_LEFT_ARROW,
    ESCAPE_CODE_RIGHT_ARROW,
    ESCAPE_CODE_CTRL_C,
    ESCAPE_CODE_CTRL_D,
    ESCAPE_CODE_CTRL_Z
} ControlCode;

// Message structure for encapsulating message details
// typedef struct {
//     ResponseCode status_code;   // Status code indicating success, failure, etc.
//     uint16_t content_length;    // Length of the message content
//     char content[512];          // Message content (up to 512 bytes for simplicity)
// } Message;

typedef struct {
    ResponseCode status_code;   // Existing status code
    ControlCode control_code;     // New field for escape codes
    uint16_t content_length;    // Length of the message content
    char content[512];          // Message content
} Message;


// Function prototypes for encoding, decoding, sending, and receiving messages
int send_message(int client_fd, const Message *msg);
int receive_message(int socket_fd, Message *msg);

#endif // PROTOCOL_H