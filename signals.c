/**
 * @file signals.c
 * @brief Signal handling for the shell
 *
 * This file contains functions to handle signals like SIGINT, SIGQUIT, etc.
 * 
 */

#include "signals.h"
#include "simple_shell.h"
#include "terminal.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

void setup_signal_handlers() {
    struct sigaction sa_int, sa_chld, sa_quit, sa_tstp;

    // Setup for SIGINT (CTRL-C)
    sa_int.sa_handler = &handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("sigaction SIGINT");
        exit(1);
    }

    // Setup for SIGQUIT (CTRL-\)
    sa_quit.sa_handler = &handle_sigquit;
    sigemptyset(&sa_quit.sa_mask);
    sa_quit.sa_flags = SA_RESTART;
    if (sigaction(SIGQUIT, &sa_quit, NULL) == -1) {
        perror("sigaction SIGQUIT");
        exit(1);
    }

    // Setup for SIGTSTP (CTRL-Z)
    sa_tstp.sa_handler = &handle_sigtstp;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = SA_RESTART;
    if (sigaction(SIGTSTP, &sa_tstp, NULL) == -1) {
        perror("sigaction SIGTSTP");
        exit(1);
    }

    // Setup for SIGCHLD
    sa_chld.sa_handler = &handle_sigchld;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa_chld, NULL) == -1) {
        perror("sigaction SIGCHLD");
        exit(1);
    }
}

void handle_sigchld(int sig)
{
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        printf("[Process %d exited]\n", pid); // TODO remove this later????
    }
}

void handle_sigint(int sig) {
    printf("\nCannot terminate the shell using CTRL-C.\n%s", PS1);
    fflush(stdout);
}

void handle_sigquit(int sig) {
    printf("\nCannot quit the shell using CTRL-\\.\n%s", PS1);
    fflush(stdout);
}

void handle_sigtstp(int sig) {
    printf("\nCannot suspend the shell using CTRL-Z.\n%s", PS1);
    fflush(stdout);
}

