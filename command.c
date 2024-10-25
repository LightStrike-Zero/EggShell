#include "command.h"


// Initialize the Command struct
void init_command(Command *cmd) {
    cmd->original_command = NULL;
    cmd->command_name = NULL;
    memset(cmd->args, 0, sizeof(cmd->args));
    cmd->arg_count = 0;
    cmd->is_background = 0;
    cmd->input_redirection = NULL;
    cmd->output_redirection = NULL;
    cmd->error_redirection = NULL;
    cmd->append_output = 0;
    cmd->next = NULL;
}

// Free memory allocated in Command struct
void free_command(Command *cmd) {
    if (cmd == NULL) return;
    free(cmd->original_command);
    // Free each argument
    for (int i = 0; i < cmd->arg_count; i++) {
        free(cmd->args[i]);
    }
    free(cmd->input_redirection);
    free(cmd->output_redirection);
    free(cmd->error_redirection);
    free_command(cmd->next);
    free(cmd->next);
}

// Helper function to handle wildcard expansion
void expand_wildcards(char *token, Command *cmd) {
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));
    int ret = glob(token, GLOB_TILDE, NULL, &glob_result);
    if (ret == 0) {
        for (size_t i = 0; i < glob_result.gl_pathc; ++i) {
            cmd->args[cmd->arg_count++] = strdup(glob_result.gl_pathv[i]);
        }
        globfree(&glob_result);
    } else {
        // No matches found, keep the token as is
        cmd->args[cmd->arg_count++] = strdup(token);
    }
}

// Parse individual command string into Command struct
int parse_command_string(char *input, Command *cmd) {
    init_command(cmd);
    cmd->original_command = strdup(input);

    char *tokens[MAX_TOKENS];
    int num_tokens = tokenise(input, tokens);
    int i = 0;

    int in_redirection = 0, out_redirection = 0, err_redirection = 0;

    while (i < num_tokens) {
        char *token = tokens[i];

        if (strcmp(token, "<") == 0) {
            in_redirection = 1;
        } else if (strcmp(token, ">") == 0) {
            out_redirection = 1;
            cmd->append_output = 0;
        } else if (strcmp(token, ">>") == 0) {
            out_redirection = 1;
            cmd->append_output = 1;
        } else if (strcmp(token, "2>") == 0) {
            err_redirection = 1;
        } else if (strcmp(token, "|") == 0) {
            // Handle pipeline
            if (i + 1 >= num_tokens) {
                fprintf(stderr, "Error: Expected command after '|'\n");
                return -1;
            }
            cmd->next = malloc(sizeof(Command));
            if (cmd->next == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
            // Construct remaining input for the next command
            char remaining_input[MAX_COMMAND_LENGTH] = "";
            for (int j = i + 1; j < num_tokens; j++) {
                strcat(remaining_input, tokens[j]);
                if (j < num_tokens - 1) {
                    strcat(remaining_input, " ");
                }
            }
            if (parse_command_string(remaining_input, cmd->next) != 0) {
                fprintf(stderr, "Error parsing piped command\n");
                return -1;
            }
            break;
        } else if (in_redirection) {
            cmd->input_redirection = strdup(token);
            in_redirection = 0;
        } else if (out_redirection) {
            cmd->output_redirection = strdup(token);
            out_redirection = 0;
        } else if (err_redirection) {
            cmd->error_redirection = strdup(token);
            err_redirection = 0;
        } else {
            // Normal argument
            cmd->args[cmd->arg_count++] = strdup(token);
        }
        i++;
    }
    cmd->args[cmd->arg_count] = NULL;

    if (cmd->arg_count > 0) {
        cmd->command_name = cmd->args[0];
    } else if (cmd->next == NULL) {
        // No command to execute
        return -1;
    }

    // Free tokens
    for (int k = 0; k < num_tokens; k++) {
        free(tokens[k]);
    }

    return 0;
}