#include "token.h"
#include <glob.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

void trim_whitespace(char *str)
{
    char *end;
    while (isspace((unsigned char)*str))
        str++;

    if (*str == 0)
    {
        *str = '\0';
        return;
    }
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;
    *(end + 1) = '\0';
}

int expand_wildcard(char *pattern, char *expanded_args[], int max_args)
{
    glob_t glob_result;
    int i, count = 0;

    memset(&glob_result, 0, sizeof(glob_result));

    // Wildcard expansion using glob
    if (glob(pattern, GLOB_TILDE, NULL, &glob_result) == 0)
    {
        // Copy the expanded file paths into expanded_args
        for (i = 0; i < glob_result.gl_pathc && count < max_args; i++)
        {
            expanded_args[count] = strdup(glob_result.gl_pathv[i]);
            if (expanded_args[count] == NULL)
            {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
            count++;
        }
    }
    else
    {
        // No matches found, return 0 to indicate no expansion
        count = 0;
    }

    // Free glob memory
    globfree(&glob_result);

    return count;
}

int tokenise(char *line, char *tokens[])
{
    int i = 0;
    char *ptr = line;

    while (*ptr != '\0')
    {
        // Skip leading whitespace
        while (isspace((unsigned char)*ptr))
        {
            ptr++;
        }
        if (*ptr == '\0')
            break;

        char *token = malloc(MAX_TOKEN_LENGTH);
        if (token == NULL)
        {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
        int j = 0;
        int in_single_quote = 0;
        int in_double_quote = 0;

        while (*ptr != '\0' && (in_single_quote || in_double_quote || !isspace((unsigned char)*ptr)))
        {
            if (*ptr == '\\')
            {
                ptr++;
                if (*ptr != '\0')
                {
                    token[j++] = *ptr++;
                }
            }
            else if (*ptr == '\'' && !in_double_quote)
            {
                in_single_quote = !in_single_quote;
                ptr++; // Skip the quote
            }
            else if (*ptr == '\"' && !in_single_quote)
            {
                in_double_quote = !in_double_quote;
                ptr++; // Skip the quote
            }
            else
            {
                token[j++] = *ptr++;
            }

            if (j >= MAX_TOKEN_LENGTH - 1)
            {
                fprintf(stderr, "Token too long\n");
                exit(1);
            }
        }
        token[j] = '\0';

        if (j > 0)
        {
            if (i >= MAX_TOKENS)
            {
                fprintf(stderr, "Too many tokens\n");
                exit(1);
            }

            // Check for wildcards in the token
            if (strpbrk(token, "*?") != NULL)
            {
                // Perform wildcard expansion
                glob_t glob_result;
                memset(&glob_result, 0, sizeof(glob_result));

                int glob_ret = glob(token, GLOB_TILDE, NULL, &glob_result);
                if (glob_ret == 0)
                {
                    // Copy expanded tokens into tokens array
                    for (size_t k = 0; k < glob_result.gl_pathc; k++)
                    {
                        if (i >= MAX_TOKENS)
                        {
                            fprintf(stderr, "Too many tokens after expansion\n");
                            exit(1);
                        }
                        tokens[i++] = strdup(glob_result.gl_pathv[k]);
                    }
                    free(token); // Original token is no longer needed
                }
                else
                {
                    // No matches found; keep the token as is
                    tokens[i++] = token;
                }

                globfree(&glob_result);
            }
            else
            {
                // No wildcard; add token as is
                tokens[i++] = token;
            }
        }
        else
        {
            free(token);
        }
    }
    tokens[i] = NULL;
    return i;
}