#ifndef SIGNALS_H
#define SIGNALS_H

void setup_signal_handlers();
void handle_sigchld(int sig);
void handle_sigint(int sig);
void handle_sigquit(int sig);
void handle_sigtstp(int sig);

#endif // SIGNALS_H