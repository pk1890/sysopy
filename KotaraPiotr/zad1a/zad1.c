#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#define TIME_FMT "%Y.%m.%d %H:%M:%S"


void stopHandle();
void contHandle();

short isStopped = 0;

void sigintHandle(){
    printf("Przechwycono ctrl+c\n");
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

void contHandle(){
    resetSignals();
    printf("Wznawiam\n");
}

void stopHandle(){
    isStopped = 1;
    printf("Oczekuje na Ctrl+z - kontynuacja lub Ctrl-c - zakonczenie programu\n");
    signal(SIGTSTP, contHandle);

    struct sigaction act;
    act.sa_handler = SIG_DFL;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGINT, &act, NULL);
}

int main(){
    char buff [256];

    resetSignals();


    while(1){
        if(!isStopped){
            time_t currtime;
			struct tm *timeinfo;
			time(&currtime);
			timeinfo = localtime(&currtime);
			strftime(buff, 256, TIME_FMT, timeinfo);
            printf("%s \n", buff);
        }

        sleep(1);
    }



    return 0;
}