
#define MAX_ARGS 100
#define MAX_TOKENS 1000
#define TOKEN_SEPARATORS " \t\r\n"

int tokenise(char line[], char *token[]);
int expand_wildcard(char *pattern, char *expanded_args[], int max_args);
