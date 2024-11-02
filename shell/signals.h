/**
 * @file signals.h
 * @brief Header file for signal handling functions
 * 
 * @author Shaun Matthews & Louise Barjaktarevic
 * @date 02/10/24
 */


#ifndef SIGNALS_H
#define SIGNALS_H

    /**
     * @brief Sets up signal handlers for specific signals (e.g., SIGINT, SIGCHLD).
     */
void setup_signal_handlers();

    /**
     * @brief Handles the SIGCHLD signal, indicating a child process has terminated.
     * @param sig The signal number (typically SIGCHLD).
     */
void handle_sigchld(int sig);

    /**
     * @brief Handles the SIGINT signal (e.g., Ctrl+C) to prevent shell termination.
     * @param sig The signal number (typically SIGINT).
     */
void handle_sigint(int sig);

    /**
     * @brief Handles the SIGQUIT signal (e.g., Ctrl+\) to prevent shell termination.
     * @param sig The signal number (typically SIGQUIT).
     */
void handle_sigquit(int sig);

    /**
     * @brief Handles the SIGTSTP signal (e.g., Ctrl+Z) to prevent shell suspension.
     * @param sig The signal number (typically SIGTSTP).
     */
void handle_sigtstp(int sig);

#endif // SIGNALS_H