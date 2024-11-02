// TODO add author information and brief description

#ifndef BUILTINS_H
#define BUILTINS_H

/* Function Declarations */
//TODO add comments to the function declarations
void change_directory(const char *path);
void cd(const char *path);
void pwd();
void connect_to_server(char *hostname, const int port);
void man();
void set_prompt(const char *new_prompt);
void exit_shell();

#endif // BUILTINS_H