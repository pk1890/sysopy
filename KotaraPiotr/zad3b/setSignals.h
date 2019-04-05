#ifndef SETSIGNALS_H
#define SETSIGNALS_h

#define WAIT_FOR_RESOPONSE 1
#define RECIEVE 2

#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


void setSigHandlers(void (*handle_func)(int, siginfo_t*, void*), int siginfo, int sigstop);

void exiterror(char* err);

#endif