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
void clientConnect(const char *hostname);
void man();
void change_hostname();
void exit_shell();

#endif // BUILTINS_H