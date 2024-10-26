#ifndef FORMATTING_H
#define FORMATTING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/ioctl.h> // contains struct for winsize. to store the size of the users terminal. 


// void get_terminal_size(int *rows, int *cols); // gets size of users terminal. 
void welcome_message(int width); // displays welcome message, scaled to fit the users terminal size. 
// why? because i want to display ascii art, and if its not scaled it looks like a dreadful mess of back slashes. 


#endif 
