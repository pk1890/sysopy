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
int wait_for_response = WAIT_FOR_RESOPONSE;
pid_t pid;
char* mode;
int i = 0;

void summary(){
    printf("Sent: %d, Recieved: %d", sent, count);
    exit(0);
}

void handleSigs(int signo, siginfo_t *info, void* ucontext){
    if(wait_for_response == WAIT_FOR_RESOPONSE){
        if(signo == siginfo){
            //  printf("Recieved confirmation\n");
            if(i < sent)
            {
                // printf("Sending next\n");
                if(strncmp(mode, "kill", 4) == 0){
                kill(pid, siginfo);
                }
                else if(strncmp(mode, "queue", 5) == 0){
                    union sigval sv = {i};
                    isqueue = 1;
                    sigqueue(pid, siginfo, sv);
                }
                else if(strncmp(mode, "rt", 2) == 0){
                    kill(pid, siginfo);   
                }
                else exiterror("Bad args\n");
                i++;
                // printf("waiting for confirm\n");
            }else{
                
                kill(pid, sigstop);
                // printf("Sending stop signal\n");
                wait_for_response = RECIEVE;
            }
        }
    }
    else{
        if(signo == siginfo){
            count++;
            kill(pid, siginfo);
            // printf("sending confirmation\n");
            if(isqueue)
                printf("%dth SIGUSR1 caught\n", info->si_value.sival_int);
        }
        else if (signo == sigstop) summary();
    }
}




int main(int argc, char **argv){
    if(argc != 4) exiterror("Bad num of args!\n");
    pid = atoi(argv[1]);
    sent = atoi(argv[2]);
    mode = argv[3];
    printf("sending %d sigs\n", sent);
    if(strncmp(argv[3], "rt", 2) == 0){
        siginfo = SIGRTMAX;
        sigstop = SIGRTMIN;
    }else{
        siginfo = SIGUSR1;
        sigstop = SIGUSR2;   
    }
    setSigHandlers(handleSigs, siginfo, sigstop);

    if(sent == 0) exit(0);

    i++;

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
    printf("waiting for confirm\n");
   while(1){
       pause();
   }
    
}