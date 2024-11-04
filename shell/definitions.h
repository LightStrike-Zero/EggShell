/**
 * @file definitions.h
 * @brief Header file for various defines used throughout the shell
 *
 * 
 * @author Shaun Matthews & Louise Barjaktarevic
 * @date 16/10/24
 */

#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define MAX_ARGS            1000
#define MAX_TOKENS          1000
#define MAX_TOKEN_LENGTH    1024
#define HISTORY_SIZE        100
#define MAX_COMMAND_LENGTH  1024
#define MAX_PROMPT_LENGTH   1024
#define MAX_PROMPT_LENGTH   1024

/* escape characters for keys */
#define BACKSPACE 127
#define CTRL_D 4
#define ESCAPE 27

/* Defines for the color types used in the shell */
#define MAGENTA         "\033[35m" 
#define BRIGHT_MAGENTA  "\033[95m"
#define PINK            "\033[38;5;200m"
#define BRIGHT_PINK     "\033[38;5;205m" 
#define HOT_PINK        "\033[38;5;209m" 
#define RED             "\033[31m"
#define GREEN           "\033[32m"
#define LIGHT_GREEN     "\033[38;5;118m"
#define YELLOW          "\033[33m"
#define BLUE            "\033[34m"  
#define SKY_BLUE        "\033[38;5;117m" 
#define BRIGHT_BLUE     "\033[38;5;81m"
#define DARK_BLUE       "\033[38;5;19m"   
#define NAVY            "\033[38;5;19m" 
#define TEAL            "\033[38;5;37m"  
#define CYAN            "\033[36m"
#define BRIGHT_ORANGE   "\033[38;5;214m" 
#define DARK_ORANGE     "\033[38;5;202m" 
#define RESET           "\033[0m"


#define PORT 42010

extern char PS1[MAX_COMMAND_LENGTH];

#endif // DEFINITIONS_H