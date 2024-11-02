
/* Project Includes */
#include "token.h"

/* System Includes */
#include <glob.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

/* Function Definitions */

void trim_whitespace(char *string) // trims all white space between tokens. 
{
    while (isspace((unsigned char)*string))
        string++;

    if (*string == 0)
    {
        *string = '\0';
        return;
    }
    char* end = string + strlen(string) - 1;
    while (end > string && isspace((unsigned char)*end))
        end--;
    *(end + 1) = '\0';
}

int expand_wildcard(const char *pattern, char *expanded_args[], const int max_args)
{
    glob_t glob_result;
    int count = 0;

    memset(&glob_result, 0, sizeof(glob_result));

    // wildcard expansion using glob
    if (glob(pattern, GLOB_TILDE, NULL, &glob_result) == 0)
    {
        // copy the expanded file paths into expanded_args
        for (int i = 0; i < glob_result.gl_pathc && count < max_args; i++)
        {
            expanded_args[count] = strdup(glob_result.gl_pathv[i]);
            if (expanded_args[count] == NULL)
            {
                fprintf(stderr, "Memory allocation failed for wildcard expansion\n");
                exit(1);
            }
            count++;
        }
    }
    else
    {
        // no matches found, return 0 to indicate no expansion
        count = 0;
    }

    // free glob memory
    globfree(&glob_result);

    return count;
}

int tokenise(const char *line, char *tokens[])
{
    int i = 0;
    const char *currentCharPos  = line;

    while (*currentCharPos  != '\0')
    {
        // skip leading whitespace/s
        while (isspace((unsigned char)*currentCharPos ))
        {
            currentCharPos ++;
        }
        if (*currentCharPos  == '\0')
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

        while (*currentCharPos  != '\0' && (in_single_quote || in_double_quote || !isspace((unsigned char)*currentCharPos )))
        {
            if (*currentCharPos  == '\\')
            {
                currentCharPos ++;
                if (*currentCharPos  != '\0')
                {
                    token[j++] = *currentCharPos ++;
                }
            }
            else if (*currentCharPos  == '\'' && !in_double_quote)
            {
                in_single_quote = !in_single_quote;
                currentCharPos ++; // Skip the quote
            }
            else if (*currentCharPos  == '\"' && !in_single_quote)
            {
                in_double_quote = !in_double_quote;
                currentCharPos ++; // Skip the quote
            }
            else
            {
                token[j++] = *currentCharPos ++;
            }

            if (j >= MAX_TOKEN_LENGTH - 1)
            {
                fprintf(stderr, "Token too long\n");
                exit(1);
            }
        }
        token[j] = '\0';

        // Perform variable expansion if token starts with $
        if (token[0] == '$' && strlen(token) > 1)
        {
            // Remove the $ to get the environment variable name
            const char *env_var_name = token + 1;
            const char *env_var_value = getenv(env_var_name);

            if (env_var_value != NULL)
            {
                // Replace token with the environment variable's value
                free(token);
                token = strdup(env_var_value);
                if (token == NULL)
                {
                    fprintf(stderr, "Memory allocation failed\n");
                    exit(1);
                }
            }
            else
            {
                // if variable not found, replace with an empty string
                free(token);
                token = strdup("");
            }
        }

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
                glob_t glob_result = {0};

                const int glob_result_code = glob(token, GLOB_TILDE, NULL, &glob_result);
                if (glob_result_code == 0)
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