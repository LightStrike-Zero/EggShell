/**
 * @file terminal.h
 * @brief Header file for managing the state of the terminal
 * 
 * @author Shaun Matthews & Louise Barjaktarevic
 * @date 19/10/24
 */


#ifndef TERMINAL_H
#define TERMINAL_H

/* Project Includes */
#include <termios.h>

/* Global Variables */
extern struct termios original_terminal_input;

/* Function Declarations */
/**
 * @brief set the terminal to raw mode disabling "canoical" input
 *        This mode allows for character-by-character input handling
 */
void make_raw_terminal();

/**
 * @brief restores the terminal to its original settings and exiting raw mode
 */
void restore_terminal();

#endif // TERMINAL_Hs