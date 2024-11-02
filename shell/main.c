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
                // #include "definitions.h"
#include "terminal.h"
#include "signals.h"
#include "command.h"
                // #include "token.h"
#include "history.h"
#include "formatting.h"

/* System includes */
                // #include <stdlib.h> // for malloc, NULL, exit
                // #include <stdio.h>  // for printf, snprintf
#include <string.h> // for strlen, strcpy, strcmp
                // #include <dirent.h> // for DIR, opendir, readdir, closedir
                // #include <unistd.h>
                // #include <sys/types.h>
                // #include <limits.h>
                // #include <sys/wait.h>
                // #include <ctype.h>
                // #include <errno.h>

// defined in definitions.c and allocated here.
char PS1[MAX_COMMAND_LENGTH] = "%";

int main()
{
    int rows, cols; // for terminal size. 

    // set up signal handlers
    setup_signal_handlers();

    get_terminal_size(&rows, &cols);

    // display the welcome message
    // size depends on terminal size, takes cols to check users window size, and decides which message to display
    welcome_message(cols);
    while (1)
    {
        char command[MAX_COMMAND_LENGTH];
        // Read user input
        read_command(command);

        // Execute commands
        if (strlen(command) > 0)
        {
            parse_commands(command);
        }
    }


// free resources
    return 0;
}