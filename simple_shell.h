#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 100

#define BACKSPACE 127
#define CTRL_D 4
#define ESCAPE 27

#define END_PINK   "\033[0m" 
#define PINK    "\033[1;35m"

// original command line prompt is set
// var name PS1 to mimic og Unix shell
char PS1[MAX_COMMAND_LENGTH] = "[374-shell] $ ";


struct termios original_terminal_input; // global for defualt terminal attributes


void read_command(char *command);
void make_raw_terminal();
void cd(char *path);
void restore_terminal();
void pwd();
void man();
void execute_command(char *command, int is_background);
void parse_commands(char *input_command);
void setup_signal_handlers();
void handle_sigchld(int sig);
void handle_sigint(int sig);
void handle_sigquit(int sig);