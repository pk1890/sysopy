

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

belt *b;

void delBelt(){
    if(b) deleteBelt(b);
}

void int_handler(int _signum){
    b->isTruckerDone = 1;
    sleep(1);
    deleteBelt(NULL);
}


int main(int argc, char* args[]){
    if (argc != 4) exitError("Usage: trucker <capacity>, <belt_size> <belt_max_mass>"); 
    atexit(delBelt);

    int capacity = secureAtoi(args[1]); 
    int belt_size = secureAtoi(args[2]); 
    int belt_max_mass = secureAtoi(args[3]); 

	struct sigaction sa;
	sa.sa_handler = int_handler;
	sigemptyset(&sa.sa_mask);

	if (sigaction(SIGINT, &sa, NULL) == -1) exitErrno("Error in masking SIGINT");
    b = createBelt(belt_size, belt_max_mass); 
    if(b == NULL) exitError("Error in creating belt");

    truck t;
    t.max_capacity = capacity;
    t.capacity = capacity;
    while(1){
        getPackageFromBelt(b, &t);
    }
    deleteBelt(NULL);    
}

