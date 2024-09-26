#include "token.h"
#include <glob.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int expand_wildcard(char *pattern, char *expanded_args[], int max_args)
{
    glob_t glob_result;
    int i, count = 0;

    memset(&glob_result, 0, sizeof(glob_result));

    if (glob(pattern, GLOB_TILDE, NULL, &glob_result) == 0)
    {
        for (i = 0; i < glob_result.gl_pathc && count < max_args; i++)
        {
            expanded_args[count] = strdup(glob_result.gl_pathv[i]);
            count++;
        }
    }
    else
    {
        // No matches found, return 0 to indicate no expansion
        // TODO add print statement here
        count = 0;
    }

    // free glob memory
    globfree(&glob_result);

    return count;
}

int tokenise(char line[], char *token[])
{
    int i = 0;
    char *newToken = strtok(line, TOKEN_SEPARATORS);

    while (newToken != NULL)
    {
        if (i >= MAX_TOKENS)
        {
            return -1; // Too many tokens
        }

        // check if token contains a wildcard character
        if (strchr(newToken, '*') || strchr(newToken, '?'))
        {
            int expanded = expand_wildcard(newToken, &token[i], MAX_TOKENS - i);
            
            // If no wildcard expansion was found, treat the token as-is
            if (expanded == 0)
            {
                token[i] = newToken; 
                i++;
            }
            else
            {
                i += expanded; 
            }
        }
        else
        {
            // Normal token, add to the token array
            token[i] = newToken;
            i++;
        }

        // Move to the next token
        newToken = strtok(NULL, TOKEN_SEPARATORS);
    }

    token[i] = NULL; 

    return i; 
}
