#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#define TIME_FMT "%Y.%m.%d %H:%M:%S"

pid_t childPid;

void stopHandle();
void contHandle();
pid_t forkChild();

void exitError(char* err){
    printf(err);
    exit(1);
}

short isStopped = 0;

void sigintHandle(){
    printf("Przechwycono ctrl+c\n");
    if(isStopped == 0){
        kill(childPid, SIGKILL);
    }
    exit(1);
}

void resetSignals(){
    isStopped = 0;

    struct sigaction act;
    act.sa_handler = sigintHandle;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);

    signal(SIGTSTP, stopHandle);
} 

void stopHandle(){
    if(isStopped == 0){
        isStopped = 1;
        printf("Oczekuje na Ctrl+z - kontynuacja lub Ctrl-c - zakonczenie programu\n");
        kill(childPid, SIGINT);

        struct sigaction act;
        act.sa_handler = SIG_DFL;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGINT, &act, NULL);

        signal(SIGTSTP, stopHandle);
    }else
    {
        printf("Wznawiam\n");
        childPid = forkChild();
        resetSignals();
    }
    

}


pid_t forkChild(){
    pid_t forkPid = fork();
    if(forkPid == 0){
        execl("./date.sh", "date.sh" ,NULL);
    }
    return forkPid;
}

int main(){
    resetSignals();

    childPid = forkChild();
    if(childPid < 0) exitError("Error in forking new process\n");

    while(1){
        pause();
    }

    return 0;
}