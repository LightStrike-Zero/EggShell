/**
* @file server.h
* @brief Server implementation for custom Telnet-like client-server
 *
 * This server listens on a specified port, accepts incoming client connections,
 * allocates PTYs, spawns shell processes, and relays data between clients and shells.
 *
 * @author Shaun Matthews & Louise Barjaktarevic
 * @date 30/10/24
 */


#ifndef SERVER_H
#define SERVER_H

#include "../protocol.h"

/* Constants */
#define DEFAULT_PORT 40210
#define BACKLOG 10          // Number of pending connections queue will hold
#define BUFFER_SIZE 4096    // Buffer size for data relay

#define RESET           "\033[0m"
#define LIGHT_GREEN     "\033[38;5;118m"
#define RED             "\033[31m"


/* Function Declarations */
void setup_server(int *server_fd, const int port);
void handle_client(const int client_fd);
void relay_data(const int master_fd, const int client_fd);
void reap_zombie_processes(const int sig);
void setup_signal_handlers();
void log_event(const char *format, ...);
int load_users();
int authenticate_user(const char *username, const char *password);
void send_response(int client_fd, ResponseCode response_code, const char *message);

#endif //SERVER_H
