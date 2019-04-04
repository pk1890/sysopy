#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include"setSignals.h"

int count = 0;
int sent;
int siginfo;
int sigstop;
int isqueue = 0;

void summary(){
    printf("Sent: %d, Recieved: %d", sent, count);
    exit(0);
}

void handleSigs(int signo, siginfo_t *info, void* ucontext){
    if(signo == siginfo){
        count++;
        if(isqueue)
            printf("%dth SIGUSR1 caught\n", info->si_value.sival_int);
    }
    else if (signo == sigstop) summary();
}




int main(int argc, char **argv){
    if(argc != 4) exiterror("Bad num of args!\n");
    pid_t pid = atoi(argv[1]);
    sent = atoi(argv[2]);
    printf("sending %d sigs\n", sent);
    if(strncmp(argv[3], "rt", 2) == 0){
        siginfo = SIGRTMAX;
        sigstop = SIGRTMIN;
    }else{
        siginfo = SIGUSR1;
        sigstop = SIGUSR2;   
    }
    setSigHandlers(handleSigs, siginfo, sigstop);
    for(size_t i = 0; i < sent; i++)
    {
        if(strncmp(argv[3], "kill", 4) == 0){
           kill(pid, siginfo);
        }
        else if(strncmp(argv[3], "queue", 5) == 0){
            union sigval sv = {i};
            isqueue = 1;
            sigqueue(pid, siginfo, sv);
        }
        else if(strncmp(argv[3], "rt", 2) == 0){
            kill(pid, siginfo);   
        }
        else exiterror("Bad args\n");
    }
   kill(pid, sigstop);
   while(1){
       pause();
   }
    
}