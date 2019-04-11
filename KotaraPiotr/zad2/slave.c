#include <fcntl.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_LEN 256

char date[128];
char buff[256];

void exitError(char* msg){
    printf("%s\n", msg);
    exit(1);
}
int main(int argc, char **argv){
    if(argc != 3) exitError("Bad no of args!");

    char *path = argv[1];
    int N = atoi(argv[2]);

    printf("%d\n", getpid());

    int pipe = open(path, O_WRONLY);
    printf("Opening pipe\n");
    for(size_t i = 0; i < N; i++)
    {
        FILE* datef = popen("date", "r");
        fgets(date, MAX_LEN, datef);
        snprintf(buff, MAX_LEN, "[%d]: %s", (int)getpid(), date);
        printf("%s\n", buff);
        write(pipe, buff, MAX_LEN);
        sleep(rand()%4+2);
        pclose(datef);
    }
    close(pipe);

    
}