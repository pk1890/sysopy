#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#define _GNU_SOURCE

#define MAXARGS 10


void exitError(char* msg){
    printf("%s\n", msg);
    exit(1);
}

typedef struct{
    char* name;
    char** argv;
    int argc;
    int infd;
    int outfd;
    int inPipeWriteFd;
    int outPipeReadFd;
}command;


command parseCommand(char* comm){
    command c;
    c.name = strtok(comm, " ");
    c.argv = calloc(MAXARGS+1, sizeof(char*));
    c.argv[0] = c.name;
    c.infd = STDIN_FILENO;
    c.outfd = STDOUT_FILENO;
    char* token;
    int i = 1;
    while(i < MAXARGS && (token = strtok(NULL, " "))){
        c.argv[i] = token;
        i++;
    }
    if(i == 10) exitError("too many args");

    c.argc = i;

    return c;
    
}

pid_t launchComm(command c){
    pid_t pid = fork();
    if(pid == 0){
        if(c.inPipeWriteFd != -1){
            close(c.inPipeWriteFd);
        }
        if(c.outPipeReadFd != -1){
            close(c.outPipeReadFd);
        }
        printf("%s: %d, %d, %d, %d\n", c.name, c.inPipeWriteFd, c.infd, c.outfd, c.outPipeReadFd);
        if(c.infd != STDIN_FILENO) {
            dup2(c.infd, STDIN_FILENO);
            close(c.infd);
        }
        if(c.outfd != STDOUT_FILENO){
            dup2(c.outfd, STDOUT_FILENO);
            close(c.outfd);
        }
        execvp(c.name, c.argv);
    }else if(pid < 0 ) exitError("Error in forking");
    else return pid;
}

command* parseFile(char* path){
    FILE* fp = fopen(path, "r");

    size_t file_len = 0;

    fseek(fp, 0, SEEK_END);
    file_len = ftell(fp);
    printf("File len: %ld\n", file_len);
    rewind(fp);
    char* fileBuf = calloc(file_len, sizeof(char));
    fread(fileBuf, sizeof(char), file_len, fp);

    char* saveLinePtr;
    char* comm;
    char* line;
    char *savePipePtr;
    int first = 1;
    line = __strtok_r(fileBuf, "\n", &saveLinePtr);
    int pipedes[2];
    int descriptors[2];
    pid_t pid;
    while(line != NULL){
        printf("NEW LINE===========================\n");

        comm = __strtok_r(line, "|", &savePipePtr);

        descriptors[0] = -1;
        descriptors[1] = -1;
        while(comm != NULL){
            // printf("Comm  : %s\n", comm);
            command c = parseCommand(comm);
            printf("Comm name: %s, comm arg1: %s, arg2: %s\n", c.name, c.argv[0], c.argv[1]);
            comm = __strtok_r(NULL, "|", &savePipePtr);
            if(first){
                first = 0;
                c.inPipeWriteFd = -1;
            }
            else{
                c.inPipeWriteFd = descriptors[0];
                c.infd = descriptors[1];

            }
            if(comm == NULL) {
               c.outPipeReadFd = -1; 
            }
            else{
                pipe(pipedes);
                c.outfd = pipedes[1];
                c.outPipeReadFd = pipedes[0];
            }
            pid = launchComm(c);
            if(descriptors[0] != -1) close(descriptors[0]);
            if(descriptors[1] != -1) close(descriptors[1]);

            descriptors[0] = pipedes[1];
            descriptors[1] = pipedes[0];
            
        }
        waitpid(pid, NULL, 0);
        // printf("Comm1: %s", strtok(fileBuf, NULL));
    
        line = __strtok_r(NULL, "\n", &saveLinePtr);
    }
}

int main(){

    parseFile("test2");
    exit(0);
}

