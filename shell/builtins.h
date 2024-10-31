#ifndef BUILTINS_H
#define BUILTINS_H

#

struct Packet {
    int value1;
    int value2;
    char command[1024];
};


void change_directory(char *path);
void cd(char *path);
void pwd();
void connect_to_server(char *hostname, int port);
void man();
void change_hostname();
void exit_shell();

#endif // BUILTINS_H