//TODO add file header

#ifndef TOKEN_H
#define TOKEN_H

/* Project Includes */
#include "definitions.h"

// added file header i think you forgot shaun 
// #define MAX_ARGS 1000
// #define MAX_TOKENS 1000
// #define MAX_TOKEN_LENGTH 1024
// #define TOKEN_SEPARATORS " \t\n"

/* Function Declarations */
//TODO add comments to the function declarations
void trim_whitespace(char *string);
int tokenise(const char *line, char *tokens[]);
int expand_wildcard(const char *pattern, char *expanded_args[], const int max_args);

#endif // TOKEN_H

