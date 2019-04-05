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
int wait_for_response = RECIEVE;
char* mode;
int i = 0;

void summary(){
    printf("Sent: %d, Recieved: %d", i, count);
    exit(0);
}

void handleSigs(int signo, siginfo_t* info, void* ucontext){

    if(wait_for_response == WAIT_FOR_RESOPONSE){
        if(signo == siginfo){
            //  printf("Recieved confirmation, sending next\n");
            if(i < count)
            {
                if(strncmp(mode, "kill", 4) == 0){
                kill(sender_pid, siginfo);
                }
                else if(strncmp(mode, "queue", 5) == 0){
                    union sigval sv = {i};
                    isqueue = 1;
                    sigqueue(sender_pid, siginfo, sv);
                }
                else if(strncmp(mode, "rt", 2) == 0){
                    kill(sender_pid, siginfo);   
                }
                else exiterror("Bad args\n");
                i++;
                // printf("waiting for confirm\n");
            }else{
                // printf("Sending stop signal\n");
                kill(sender_pid, sigstop); 
                summary();
            }
        }
    }
    else{
        if(signo == siginfo){
            count++;
            sender_pid = info->si_pid;
            kill(sender_pid, siginfo);
            // printf("sending confirmation\n");
            if(isqueue)
                printf("%dth SIGUSR1 caught\n", info->si_value.sival_int);
            return;
        }
        else if (signo == sigstop){
            // printf("changing mode\n"); 
            wait_for_response = WAIT_FOR_RESOPONSE;

            if(strncmp(mode, "kill", 4) == 0){
            kill(sender_pid, siginfo);
            }
            else if(strncmp(mode, "queue", 5) == 0){
                union sigval sv = {i};
                isqueue = 1;
                sigqueue(sender_pid, siginfo, sv);
            }
            else if(strncmp(mode, "rt", 2) == 0){
                kill(sender_pid, siginfo);   
            }
            else exiterror("Bad args\n");
            i++;
            // printf("waiting for confirm\n");
        }
    }
}



int main(int argc, char **argv){
    if(argc != 2) exiterror("Bad args no");
    printf("PID: %d \n", getpid());
    mode = argv[1];
    if(strncmp(argv[1], "rt", 2) == 0){
        siginfo = SIGRTMAX;
        sigstop = SIGRTMIN;
    }else{
        siginfo = SIGUSR1;
        sigstop = SIGUSR2;   
    }
    setSigHandlers(handleSigs, siginfo, sigstop);
    while(1){
       pause();
    }

    exit(0);
     
}