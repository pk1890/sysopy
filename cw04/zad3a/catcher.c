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
pid_t sender_pid;
int waiting = 1;
int siginfo;
int sigstop;
int isqueue = 0;

void summary(){
    printf("Sent: %d, Recieved: %d", sent, count);
    exit(0);
}

void handleSigs(int signo, siginfo_t* info, void* ucontext){
    if(signo == siginfo){
        sender_pid = info->si_pid;
        count++;
        if(isqueue)
            printf("%dth SIGUSR1 caught\n", info->si_value.sival_int);
    }
    else if (signo == sigstop) {
        sender_pid = info->si_pid;
        printf("Recieved: %d\n", count);
        waiting = 0;


    }
}



int main(int argc, char **argv){
    if(argc != 2) exiterror("Bad args no");
    printf("PID: %d \n", getpid());
    if(strncmp(argv[1], "rt", 2) == 0){
        siginfo = SIGRTMAX;
        sigstop = SIGRTMIN;
    }else{
        siginfo = SIGUSR1;
        sigstop = SIGUSR2;   
    }
    setSigHandlers(handleSigs, siginfo, sigstop);
    while(waiting){
       pause();
    }
    for(size_t i = 0; i < count; i++)
    {
        if(strncmp(argv[1], "kill", 4) == 0){
           kill(sender_pid, siginfo);
        }
        else if(strncmp(argv[1], "queue", 5) == 0){
            union sigval sv = {i};
            isqueue = 1;
            sigqueue(sender_pid, siginfo, sv);
        }
        else if(strncmp(argv[1], "rt", 2) == 0){
            kill(sender_pid, siginfo);   
        }
        else exiterror("Bad args\n");
    }
    kill(sender_pid, sigstop);

    exit(0);
     
}