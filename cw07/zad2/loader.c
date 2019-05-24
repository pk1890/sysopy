
#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE 1

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "common.h"

void register_handlers();
void exit_handler();
void int_handler(int sig_no){};

int main(int argc, char* args[])
{
    if (argc < 2) exitError("Usage: loader <child_count> <{package mass}> [{lifetimes}]");

    int child_count = secureAtoi(args[1]);

    for (int i = 0; i < child_count; i++) {
        if (fork() == 0) {
            int m = secureAtoi(args[2 + i]);

            belt *b = getBelt();

            struct sigaction sa;
            sa.sa_handler = int_handler;
            sigemptyset(&sa.sa_mask);

            if (sigaction(SIGINT, &sa, NULL) == -1) exitErrno("Error in masking SIGINT");

            package pkg = createPackage(m);

            if (argc > 2 + i + child_count) {
                for (int j = 0; j < secureAtoi(args[2 + i + child_count]); j++)
                    putPkgOnBelt(b, pkg);
                closeSem(b);
            } else {
                while (1) putPkgOnBelt(b, pkg);
            }
        }
    }

    for (int i = 0; i < child_count; i++) {
        wait(NULL);
    }

    return 0;
}