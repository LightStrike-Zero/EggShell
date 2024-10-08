/**
 * @file simple_shell.c
 * @brief A basic UNIX shell implementation for ICT374
 *
 * This program implements a basic UNIX shell that reads user commands,
 * executes them by forking child processes, and handles the `exit` command to
 * terminate the shell. It also includes functionality to reclaim zombie
 * processes.
 *
 * @author Shaun Matthews & Louise Barjaktarevic
 * @date 25/09/2024
 */

/* Our includes */
#include "simple_shell.h"
#include "token.h"
#include "history.h"
#include "formatting.h"

/* System includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>

void read_command(char *command)
{
    // put the terminalin non-canonical mode to gain raw control
    make_raw_terminal();

    int index = 0;
    printf("%s", PS1);
    fflush(stdout);

    while (1)
    {
        char character;
        ssize_t nread = read(STDIN_FILENO, &character, 1);

        if (nread == -1 && errno != EAGAIN)
        {
            perror("read");
            break;
        }
        else if (nread == 0)
        {
            // EOF (Ctrl+D)
            printf("\n");
            restore_terminal();
            exit(0);
        }

        if (character == '\n')
        {
            command[index] = '\0';
            printf("\n");
            break;
        }
        else if (character == BACKSPACE || character == 8)
        {
            if (index > 0)
            {
                index--;
                command[index] = '\0';
                printf("\b \b");
                fflush(stdout);
            }
        }
        else if (character == ESCAPE)
        {
            handle_history_navigation(command, &index, PS1);
        }
        else if (isprint(character))
        {
            if (index < MAX_COMMAND_LENGTH - 1)
            {
                command[index++] = character;
                command[index] = '\0';
                printf("%c", character);
                fflush(stdout);
            }
        }
    }

    // tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_terminal_input);
    // Restore original terminal settings
    restore_terminal();

    // Add command to history if it's not empty
    if (strlen(command) > 0)
    {
        add_to_history(command);
    }
}

void make_raw_terminal()
{
    struct termios raw_terminal_input_mode;
    tcgetattr(STDIN_FILENO, &original_terminal_input);
    raw_terminal_input_mode = original_terminal_input;

    // Set raw mode
    raw_terminal_input_mode.c_lflag &= ~(ECHO | ICANON);
    raw_terminal_input_mode.c_cc[VMIN] = 1;
    raw_terminal_input_mode.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_terminal_input_mode);
}

void restore_terminal()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_terminal_input);
}

void man()
{
    printf("This is Eggshell's user manual.\n"
           "Available commands:\n"
           "Usage: <hostname> $ <command>\n"
          PINK "pwd -     Prints the working directory.\n"
           "history - Use up and down arrow keys to toggle through command history.\n"
           "cd - Print the current directory."
           "exit -    Exit shell. Bye Bye.\n"END_PINK); 
}

void pwd_recurse(Node *nodePtr)
{
    if (nodePtr == root)
    {
        return;
    }
    pwd_recurse(nodePtr->parent);
    printf("/%s", nodePtr->name); // will unravel path from cwd -> root, printing from root -> cwd!
}

void pwd()
{
    pwd_recurse(cwd); // recursive function, starts from cwd, cwd' parent, etc, etc.
    printf("\n");
}

void execute_command(char *command, int is_background)
{

    // Handle empty command first to easy out
    if (strlen(command) == 0)
    {
        return;
    }
    // Check for repeat command by number (!n)
    if (command[0] == '!' && isdigit(command[1]))
    {
        int command_number = atoi(&command[1]);
        repeat_command_by_number(command_number, command);
        if (strlen(command) == 0)
            return; // No valid command found
    }
    // Check for repeat command by string (!string)
    else if (command[0] == '!' && isalpha(command[1]))
    {
        repeat_command_by_string(&command[1], command);
        if (strlen(command) == 0)
            return; // No valid command found
    }

    char *args[MAX_ARGS];
    int num_tokens;

    // Tokenize the command
    num_tokens = tokenise(command, args);
    if (num_tokens < 0)
    {
        fprintf(stderr, "Error: Too many tokens\n");
        return;
    }

    // Handle built-in commands before forking
    if (strcmp(args[0], "exit") == 0)
    {
        restore_terminal();
        printf("Exiting shell...\n");
        exit(0);
    }
    else if (strcmp(args[0], "history") == 0)
    {
        show_history();
        return;
    }
    else if (strcmp(args[0], "pwd") == 0)
    {
        pwd();
        return;
    }

    else if (strcmp(args[0], "man") == 0)
    {
        man();
        return;
    }

    else if (strcmp(args[0], "cd") == 0)
    {
        printf("/%s", nodePtr->name);
    }
    // Add other built-in commands here...

    // Fork and execute external commands
    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return;
    }
    if (pid == 0)
    {
        // Child process
        if (is_background)
        {
            setpgid(0, 0); // Detach from the controlling terminal
        }
        if (execvp(args[0], args) < 0)
        {
            perror("execvp");
            exit(1);
        }
    }
    else
    {
        // Parent process
        if (!is_background)
        {
            waitpid(pid, NULL, 0);
        }
        else
        {
            printf("[Background PID: %d]\n", pid);
        }
    }
}

void parse_commands(char *input_command)
{
    char *commands[MAX_ARGS];
    char separators[MAX_ARGS];
    int num_commands = 0;

    // Copy input_command to a temporary buffer because strtok modifies the string
    char temp_input[MAX_COMMAND_LENGTH];
    strncpy(temp_input, input_command, MAX_COMMAND_LENGTH);
    temp_input[MAX_COMMAND_LENGTH - 1] = '\0';

    char *cmd = temp_input;
    while (cmd != NULL && num_commands < MAX_ARGS)
    {
        // Find next occurrence of ';' or '&'
        char *sep = strpbrk(cmd, ";&");
        if (sep != NULL)
        {
            separators[num_commands] = *sep;
            *sep = '\0';
            commands[num_commands++] = cmd;
            cmd = sep + 1;
        }
        else
        {
            // Last command without a following separator
            commands[num_commands++] = cmd;
            separators[num_commands - 1] = '\0';
            break;
        }
    }

    for (int i = 0; i < num_commands; i++)
    {
        char *command = commands[i];
        trim_whitespace(command);

        if (strlen(command) == 0)
        {
            continue;
        }

        // Determine if the command should run in the background
        int is_background = 0;
        if (separators[i] == '&')
        {
            is_background = 1;
        }
        else if (separators[i] == ';' || separators[i] == '\0')
        {
            is_background = 0;
        }

        // Check if the command ends with '&'
        size_t len = strlen(command);
        if (separators[i] == '\0' && len > 0 && command[len - 1] == '&')
        {
            is_background = 1;
            command[len - 1] = '\0';
            trim_whitespace(command);
        }

        // Execute the command
        execute_command(command, is_background);
    }
}

void setup_signal_handlers()
{
    struct sigaction sa_int, sa_chld, sa_quit;

    // Setup for SIGINT
    sa_int.sa_handler = &handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa_int, NULL) == -1)
    {
        perror("sigaction SIGINT");
        exit(1);
    }

    // Setup for SIGCHLD
    sa_chld.sa_handler = &handle_sigchld;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa_chld, NULL) == -1)
    {
        perror("sigaction SIGCHLD");
        exit(1);
    }

    // Setup for SIGQUIT
    sa_quit.sa_handler = &handle_sigquit;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = SA_RESTART;
    if (sigaction(SIGQUIT, &sa_quit, NULL) == -1)
    {
        perror("sigaction SIGQUIT");
        exit(1);
    }
}

void handle_sigchld(int sig)
{
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        printf("[Process %d exited]\n", pid); // TODO remove this later????
    }
}

void handle_sigint(int sig)
{
    restore_terminal();
    printf("\n");
    exit(0);
}

void handle_sigquit(int sig)
{
    restore_terminal(); // Restore original terminal settings

    // Reset SIGQUIT to its default action so that it generates a core dump incase this was cause by an error
    struct sigaction sa;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGQUIT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    // Re-raise SIGQUIT to trigger the default behavior (core dump)
    raise(SIGQUIT);
}

int main()
{
    char command[MAX_COMMAND_LENGTH];
    int rows, cols; // for terminal size. 
    Node homeDir;

    root = malloc(sizeof(Node));
    strcpy(root->name, ""); // root has no name.
    root->parent = NULL;
    // root set, with name and parent ("" and NULL).

    homeDir = malloc(sizeof(Node));
    strcpy(homeDir->name, "home");
    homeDir->parent = root;

    // Set up signal handlers
    setup_signal_handlers();

    get_terminal_size(&rows, &cols);

    // Display the welcome message
    welcome_message(cols);

    while (1)
    {
        // Read user input
        read_command(command);

        // Execute commands
        if (strlen(command) > 0)
        {
            parse_commands(command);
        }
    }

    // unreachable in current code, but good practice.
    free(cwd);
    free(root);

    return 0;
}