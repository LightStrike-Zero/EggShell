/**
 * @file token.h
 * @brief Handles tokenisation
 * 
 * @author Shaun Matthews & Louise Barjaktarevic
 * @date 02/10/24
 */


#ifndef TOKEN_H
#define TOKEN_H

/* Project Includes */
#include "definitions.h"

/* Function Declarations */
/**
 * @brief Removes leading and trailing whitespace from a string.
 * @param string The string to trim.
 */
void trim_whitespace(char *string);

/**
 * @brief Splits a line of input into tokens based on whitespace and separators.
 * @param line The input string to tokenize.
 * @param tokens Array to store the pointers to tokens.
 * @return The number of tokens found.
 */
int tokenise(const char *line, char *tokens[]);

/**
 * @brief Expands a wildcard pattern into matching file paths.
 * @param pattern The wildcard pattern to expand.
 * @param expanded_args Array to store the expanded file paths.
 * @param max_args Maximum number of arguments to store in expanded_args.
 * @return The number of expanded paths added to expanded_args.
 */
int expand_wildcard(const char *pattern, char *expanded_args[], const int max_args);



#endif // TOKEN_H

