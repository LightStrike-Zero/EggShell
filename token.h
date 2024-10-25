#ifndef TOKEN_H
#define TOKEN_H 

// added file header i think you forgot shaun 
#define MAX_ARGS 1000
#define MAX_TOKENS 1000
#define TOKEN_SEPARATORS " \t\n"

void trim_whitespace(char *str);
int tokenise(char line[], char *token[]);
int expand_wildcard(char *pattern, char *expanded_args[], int max_args);

#endif

