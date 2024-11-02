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

#define BUFFER_SIZE 4096
/* Constants */
#define DEFAULT_PORT 40210
#define BACKLOG 10          // Number of pending connections queue will hold
#define BUFFER_SIZE 4096    // Buffer size for data relay

/* Function Declarations */
void setup_server(int *server_fd, const int port);
void handle_client(const int client_fd);
void relay_data(const int master_fd, const int client_fd);
void reap_zombie_processes(const int sig);
void setup_signal_handlers();
void log_event(const char *format, ...);
int load_users();
int authenticate_user(const char *username, const char *password);

#endif //SERVER_H
