/**
 * @file main.c
 * @brief A basic UNIX shell implementation for ICT374
 *
 * This program implements a basic UNIX shell
 * 
 * 
 * @author Shaun Matthews & Louise Barjaktarevic
 * @date 25/09/2024
 */


/* Our includes */
#include "definitions.h"
#include "signals.h"
#include "command.h"

/* System includes */
#include <stdio.h>
#include <string.h> // for strlen, strcpy, strcmp



// defined in definitions.c and allocated here.
char PS1[MAX_COMMAND_LENGTH] = "%";

void welcome_message();

int main()
{
    // set up signal handlers
    setup_signal_handlers();

    // display the welcome message
    welcome_message();
    while (1)
    {
        char command[MAX_COMMAND_LENGTH];
        // read user input
        read_command(command);

        // executes commands
        if (strlen(command) > 0)
        {
            parse_commands(command);
        }
    }

    return 0; // exit 
}

void welcome_message()
{


    printf(BRIGHT_MAGENTA   " _______               "BLUE"        __           __ __  "RESET"\n");
    printf(BRIGHT_MAGENTA   "|    ___|.-----.-----. "BLUE"-----.|  |--.-----.|  |  |  "RESET"\n");
    printf(BRIGHT_MAGENTA   "|    ___||  _  |  _  | "BLUE"__ --||     |  -__||  |  |  "RESET"\n");
    printf(PINK             "|_______||___  |___  | "BRIGHT_BLUE"_____||__|__|_____||__|__|  "RESET"\n");
    printf(PINK             "         |_____|_____| "BRIGHT_BLUE"                            "RESET"\n");
    printf(BRIGHT_MAGENTA    "Developed by "PINK"Louise Barjaktarevic "BRIGHT_BLUE"& "BLUE"Shaun Matthews\n"RESET);
}