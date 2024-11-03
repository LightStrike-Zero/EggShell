//
// Created by Shaun on 3/11/2024.
//
#include "protocol.h"
#include <string.h>
#include <arpa/inet.h>  // For hton/ntoh functions

// Encodes a Message struct into a byte array for transmission
int encode_message(const Message *msg, char *buffer, size_t buffer_size) {
    if (buffer_size < sizeof(MessageType) + sizeof(uint16_t) + msg->length) {
        return -1; // Buffer too small
    }

    // Copy MessageType
    *(MessageType *)buffer = htonl(msg->type);

    // Copy length
    *(uint16_t *)(buffer + sizeof(MessageType)) = htons(msg->length);

    // Copy content
    memcpy(buffer + sizeof(MessageType) + sizeof(uint16_t), msg->content, msg->length);

    return sizeof(MessageType) + sizeof(uint16_t) + msg->length;
}

// Decodes a byte array into a Message struct
int decode_message(const char *buffer, size_t buffer_size, Message *msg) {
    if (buffer_size < sizeof(MessageType) + sizeof(uint16_t)) {
        return -1; // Buffer too small
    }

    // Extract MessageType
    msg->type = ntohl(*(MessageType *)buffer);

    // Extract length
    msg->length = ntohs(*(uint16_t *)(buffer + sizeof(MessageType)));

    if (buffer_size < sizeof(MessageType) + sizeof(uint16_t) + msg->length) {
        return -1; // Buffer size doesn't match content length
    }

    // Extract content
    memcpy(msg->content, buffer + sizeof(MessageType) + sizeof(uint16_t), msg->length);
    msg->content[msg->length] = '\0'; // Null-terminate the content

    return 0;
}