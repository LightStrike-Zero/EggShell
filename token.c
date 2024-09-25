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

    // wildcard expansion using glob
    if (glob(pattern, GLOB_TILDE, NULL, &glob_result) == 0)
    {
    
        for (i = 0; i < glob_result.gl_pathc && count < max_args; i++)
        {
            expanded_args[count] = strdup(glob_result.gl_pathv[i]);
            count++;
        }
    }


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
            return -1;
        }

        // token contains a wildcard character?
        if (strchr(newToken, '*') || strchr(newToken, '?'))
        {
            // expand the wildcard token and add expanded results to token array
            i += expand_wildcard(newToken, &token[i], MAX_TOKENS - i);
        }
        else
        {
            token[i] = newToken;
            i++;
        }

        newToken = strtok(NULL, TOKEN_SEPARATORS);
    }

    token[i] = NULL;

    return i; 
}
