/**
 * @file simple_shell.c
 * @brief A basic UNIX shell implementation for ICT374
 *
 * This program implements a basic UNIX shell
 * 
 * 
 * @author Shaun Matthews & Louise Barjaktarevic
 * @date 25/09/2024
 */

/* Our includes */
#include "simple_shell.h"
#include "command.h"
#include "token.h"
#include "history.h"
#include "formatting.h"

/* System includes */
#include <stdlib.h> // for malloc, NULL, exit
#include <stdio.h>  // for printf, snprintf
#include <string.h> // for strlen, strcpy, strcmp
#include <dirent.h> // for DIR, opendir, readdir, closedir
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>

// these are both declared in smple_shell.h but allocated here
char PS1[MAX_COMMAND_LENGTH] = "\%"; 
struct termios original_terminal_input;

void read_command(char *command)
{
    // put the terminal in non-canonical mode to gain raw control
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
    // Restore original terminal settings
    restore_terminal();
    // Add command to history if it's not empty
   if (command[0] == '!') {
        if (isdigit(command[1])) {
            int cmd_num = atoi(&command[1]);
            repeat_command_by_number(cmd_num, command);
        } else if (command[1] != '\0') {
            repeat_command_by_string(&command[1], command);
        } else {
            printf("Invalid history command.\n");
            command[0] = '\0'; // Clear the command
        }
    }

    // Add command to history if it's not empty and doesn't start with '!'
    if (strlen(command) > 0 && command[0] != '!') {
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
    // restores terminal to original settings for standard input
}
void man()
{
    // displays user manual.
    // list of available commands
    printf("This is Eggshell's user manual.\n"
           "Available commands:\n"
           "Usage: <hostname> $ <command>\n"
          PINK "pwd -     Prints the working directory.\n"
           "history - Use up and down arrow keys to toggle through command history.\n"
           "exit -    Exit shell. Bye Bye.\n"END_PINK); 
}

void change_directory(char *path) {
    if (chdir(path) < 0) {
        perror("chdir failed");
    } else {
        // Get and print the new current directory after the change
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("Changed directory to: %s\n", cwd);
        } else {
            perror("getcwd failed");
        }
    }
}

void pwd()
{
    char cwd[PATH_MAX];  // PATH_MAX is defined in <limits.h>
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("getcwd failed");
    }
}

void change_hostname()
{
    char new_hostname[MAX_COMMAND_LENGTH]; // buffer for the new hostname
    printf("Enter new hostname: ");
    if (fgets(new_hostname, sizeof(new_hostname), stdin) != NULL)
    { // Remove the newline character, if present
        int len = strlen(new_hostname);
        if (len > 0 && new_hostname[len - 1] == '\n') {
            new_hostname[len - 1] = '\0';
        }
        if (strlen(new_hostname) > 0 && strlen(new_hostname) < MAX_COMMAND_LENGTH)
        {
            snprintf(PS1, sizeof(PS1), "[%s] $", new_hostname);
            printf("Hostname has been changed.\n");
        }
        else
        {
            printf("Invalid hostname.\n");
        }
    }
    else
    {
        perror("fgets");
    }
    fflush(stdout);
}

void cd(char *path)
{
    if (path == NULL || strcmp(path, "") == 0) 
    {
        // Change to home directory if no argument is given
        const char *home_dir = getenv("HOME");
        if (home_dir == NULL) 
        {
            fprintf(stderr, "cd: HOME not set\n");
        } 
        else if (chdir(home_dir) < 0) 
        {
            perror("cd");
        }
    } 
    else if (strcmp(path, "..") == 0) 
    {
        // Move up one directory
        if (chdir("..") < 0) 
        {
            perror("cd");
        }
    } 
    else 
    {
        // Move to the specified directory
        if (chdir(path) < 0) 
        {
            perror("cd");
        }
    }


    
}

void execute_command(Command *cmd) {
    if (cmd == NULL || cmd->command_name == NULL) {
        return;
    }

    // Handle built-in commands
    if (strcmp(cmd->command_name, "exit") == 0) {
        restore_terminal();
        printf("Exiting shell...\n");
        exit(0);
    } else if (strcmp(cmd->command_name, "history") == 0) {
        show_history();
        return;
    } else if (strcmp(cmd->command_name, "pwd") == 0) {
        pwd();
        return;
    } else if (strcmp(cmd->command_name, "man") == 0) {
        man();
        return;
    } else if (strcmp(cmd->command_name, "hostname") == 0) {
        change_hostname();
        return;
    } else if (strcmp(cmd->command_name, "cd") == 0) {
        if (cmd->arg_count > 1) {
            cd(cmd->args[1]); // cd with argument
        } else {
            cd(NULL); // cd without argument
        }
        return;
    } else if (strcmp(cmd->command_name, "prompt") == 0) {
        if (cmd->arg_count > 1) {
            snprintf(PS1, sizeof(PS1), "%s", cmd->args[1]);
        } else {
            fprintf(stderr, "Usage: prompt <new_prompt>\n");
        }
        return;
    }

    int num_pipes = 0;
    Command *current_cmd = cmd;
    while (current_cmd->next != NULL) {
        num_pipes++;
        current_cmd = current_cmd->next;
    }

    int pipefds[2 * num_pipes];
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("pipe");
            return;
        }
    }

    int pid;
    int cmd_index = 0;
    current_cmd = cmd;

    while (current_cmd != NULL) {
        pid = fork();
        if (pid == 0) {
            // Child process

            // Input redirection
            if (current_cmd->input_redirection) {
                freopen(current_cmd->input_redirection, "r", stdin);
            }

            // Output redirection
            if (current_cmd->output_redirection) {
                freopen(current_cmd->output_redirection, current_cmd->append_output ? "a" : "w", stdout);
            }

            // Error redirection
            if (current_cmd->error_redirection) {
                freopen(current_cmd->error_redirection, "w", stderr);
            }

            // If not the first command, read from previous pipe
            if (cmd_index != 0) {
                if (dup2(pipefds[(cmd_index - 1) * 2], STDIN_FILENO) < 0) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            // If not the last command, write to next pipe
            if (current_cmd->next != NULL) {
                if (dup2(pipefds[cmd_index * 2 + 1], STDOUT_FILENO) < 0) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            // Close all pipe fds
            for (int i = 0; i < 2 * num_pipes; i++) {
                close(pipefds[i]);
            }

            // Execute command
            execvp(current_cmd->command_name, current_cmd->args);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else if (pid < 0) {
            perror("fork");
            return;
        }

        current_cmd = current_cmd->next;
        cmd_index++;
    }

    // Close all pipe fds in parent
    for (int i = 0; i < 2 * num_pipes; i++) {
        close(pipefds[i]);
    }

    // Wait for all child processes
    if (!cmd->is_background) {
        for (int i = 0; i <= num_pipes; i++) {
            wait(NULL);
        }
    } else {
        printf("[Background PID: %d]\n", pid);
    }
}


// Existing includes...

void parse_commands(char *input_command) {
    char *commands_str[MAX_ARGS];
    char *separators[MAX_ARGS];
    int num_commands = 0;

    // Copy input_command to a temporary buffer because strtok modifies the string
    char temp_input[MAX_COMMAND_LENGTH];
    strncpy(temp_input, input_command, MAX_COMMAND_LENGTH);
    temp_input[MAX_COMMAND_LENGTH - 1] = '\0';

    char *cmd_str = temp_input;
    while (cmd_str != NULL && num_commands < MAX_ARGS) {
        // Find next occurrence of ';' or '&' not part of command arguments
        char *sep = strpbrk(cmd_str, ";&");
        if (sep != NULL) {
            separators[num_commands] = strndup(sep, 1);
            *sep = '\0';
            commands_str[num_commands++] = cmd_str;
            cmd_str = sep + 1;
        } else {
            // Last command without a following separator
            commands_str[num_commands++] = cmd_str;
            separators[num_commands - 1] = NULL;
            break;
        }
    }

    for (int i = 0; i < num_commands; i++) {
        Command cmd;
        char *command_str = commands_str[i];
        trim_whitespace(command_str);

        if (strlen(command_str) == 0) {
            continue;
        }

        // Initialize Command struct
        if (parse_command_string(command_str, &cmd) != 0) {
            fprintf(stderr, "Failed to parse command: %s\n", command_str);
            continue;
        }

        // Determine if the command should run in the background
        if (separators[i] != NULL && strcmp(separators[i], "&") == 0) {
            cmd.is_background = 1;
        }

        // Execute the command
        execute_command(&cmd);

        // Free allocated memory
        free_command(&cmd);
        free(separators[i]);
    }
}


void setup_signal_handlers() {
    struct sigaction sa_int, sa_chld, sa_quit, sa_tstp;

    // Setup for SIGINT (CTRL-C)
    sa_int.sa_handler = &handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("sigaction SIGINT");
        exit(1);
    }

    // Setup for SIGQUIT (CTRL-\)
    sa_quit.sa_handler = &handle_sigquit;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = SA_RESTART;
    if (sigaction(SIGQUIT, &sa_quit, NULL) == -1) {
        perror("sigaction SIGQUIT");
        exit(1);
    }

    // Setup for SIGTSTP (CTRL-Z)
    sa_tstp.sa_handler = &handle_sigtstp;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    if (sigaction(SIGTSTP, &sa_tstp, NULL) == -1) {
        perror("sigaction SIGTSTP");
        exit(1);
    }

    // Setup for SIGCHLD
    sa_chld.sa_handler = &handle_sigchld;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa_chld, NULL) == -1) {
        perror("sigaction SIGCHLD");
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
void handle_sigint(int sig) {
    printf("\nCannot terminate the shell using CTRL-C.\n%s", PS1);
    fflush(stdout);
}
void handle_sigquit(int sig) {
    printf("\nCannot quit the shell using CTRL-\\.\n%s", PS1);
    fflush(stdout);
}
void handle_sigtstp(int sig) {
    printf("\nCannot suspend the shell using CTRL-Z.\n%s", PS1);
    fflush(stdout);
}

int main()
{
    char command[MAX_COMMAND_LENGTH];
    int rows, cols; // for terminal size. 


    // set up signal handlers
    setup_signal_handlers();

    get_terminal_size(&rows, &cols);

    // display the welcome message
    // size depends on terminal size, takes cols to check users window size, and decides which message to display
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


// free resources
    return 0;
}