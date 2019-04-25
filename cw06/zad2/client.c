
#include <sys/msg.h> 
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <signal.h>
#include "common.h"
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <mqueue.h>

mqd_t server_queue_d;
int ID = -1;

char cqname[256];
mqd_t client_queue_d;

void exitError(char* msg){
    printf("%s\n", msg);
    exit(1);
}

void exitErrno(char* msg){
    perror(msg);
    exit(1);
}

void deleteQueue();
void responseHandler(){
    message msg;
    if(mq_receive(client_queue_d, (char *) &msg, MAX_MESSAGE_SIZE, NULL) == -1) exitErrno("Receiving message failed");
    switch (msg.command_type)
    {
         case STOP:
                raise(SIGINT);
             break;
        case PRINT:
                printf("[%d]: %s\n", msg.sender_id, msg.text);
        default:
            break;
    }
    printf(">");
}

void sigintHandler(){
    message msg;

    msg.mtype = 1;
    msg.sender_pid = getpid();
    msg.sender_id = ID;
    msg.reciever_id = -1;
    msg.command_type = STOP;
    sprintf(msg.text, "a");

    if(mq_send(server_queue_d, (const char *) &msg, sizeof(message), 3) == -1) exitErrno("Error in sending init query");
    mq_close(client_queue_d);
    deleteQueue();
}

void deleteQueue(){
    mq_unlink(cqname);
    printf("client queue deleted succesfully\n");
    _exit(0);
}
void init(char *path){
    server_queue_d = mq_open(path, O_WRONLY);
    printf("Server Queue opened\n");
    if(server_queue_d == -1) exitErrno("Error in opening server queue");
    if(atexit(sigintHandler)) exitError("Can't initialize atexit");
    sprintf(cqname, "/clientq_%d", getpid());

    printf("%s, PID: %d\n", cqname, getpid());

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MESSAGE_SIZE;
    attr.mq_curmsgs = 0;

    client_queue_d = mq_open(cqname, O_RDONLY | O_CREAT, 0666, &attr);
    if(client_queue_d == -1) exitErrno("Error in creating client queue");


    message msg;
    msg.mtype = 1;
    msg.sender_pid = getpid();
    msg.sender_id = -1;
    msg.command_type = INIT;
    sprintf(msg.text, "%s", cqname);
    if(mq_send(server_queue_d, (const char *) &msg, sizeof(message), 1) == -1) exitErrno("Error in sending init query");

    printf("QUEUE key sent to server\n");
    if(mq_receive(client_queue_d, (char *) &msg, MAX_MESSAGE_SIZE, NULL) == -1) exitErrno("Receiving message failed");
    ID = msg.sender_id;
    printf("RECEIVED ID FROM SERVER - %d\n", ID);
    // if(msgrcv(client_queue_d, &msg, MSG_SIZE, 0, 0) < 0) exitErrno("Receiving message failed");
    // printf("Message received: \n");
    // printf("%s\n", msg.text);

}

void execEcho(char* text){

    message msg;
    msg.mtype = 3;
    msg.sender_pid = getpid();
    msg.sender_id = ID;
    msg.command_type = ECHO;
    sprintf(msg.text, text);
    if(mq_send(server_queue_d, (const char *) &msg, sizeof(msg), 1) == -1) exitErrno("Error in sending echo query");
}

void execList(){

    message msg;
    msg.mtype = 2;
    msg.sender_pid = getpid();
    msg.sender_id = ID;
    msg.command_type = LIST;
    sprintf(msg.text, "a");

    if(mq_send(server_queue_d, (const char *) &msg, sizeof(message), 1) == -1) exitErrno("Error in sending list query");
}

void exec2One(char* text, int recID){

    message msg;
    msg.mtype = 3;
    msg.sender_pid = getpid();
    msg.sender_id = ID;
    msg.reciever_id = recID;
    msg.command_type = TO_ONE;
    sprintf(msg.text, text);

    if(mq_send(server_queue_d, (const char *) &msg, sizeof(message), 1) == -1) exitErrno("Error in sending 2one query");
}
void exec2All(char* text){

    message msg;
    msg.mtype = 3;
    msg.sender_pid = getpid();
    msg.sender_id = ID;
    msg.reciever_id = -1;
    msg.command_type = TO_ALL;
    sprintf(msg.text, text);

    if(mq_send(server_queue_d, (const char *) &msg, sizeof(message), 1) == -1) exitErrno("Error in sending 2all query");
}
void execFriends(char* commandBuffer){
    message msg;
    for(int i = 0; i < MAX_CLIENTS; i++) msg.friends[i] = 0;
    size_t friend;
    char* item = strtok(commandBuffer, " ");
    while(item){
        if(sscanf(item, "%ld", &friend) == 0 || friend < 0 || friend >= MAX_CLIENTS){
            printf("Wrong friend ID: %ld\n", friend);
            return;
        }
        // printf("%s, %d\n", item, friend);
        msg.friends[friend] = 1;
        item = strtok(NULL, " ");
    }
    msg.mtype = 2;
    msg.sender_pid = getpid();
    msg.sender_id = ID;
    msg.reciever_id = -1;
    msg.command_type = FRIENDS;
    sprintf(msg.text, "a");

    if(mq_send(server_queue_d, (const char *) &msg, sizeof(message), 2) == -1) exitErrno("Error in sending friends query");
}

void execAdd(char* commandBuffer){
    message msg;
    for(int i = 0; i < MAX_CLIENTS; i++) msg.friends[i] = 0;
    size_t friend;
    char* item = strtok(commandBuffer, " ");
    while(item){
        if(sscanf(item, "%ld", &friend) == 0 || friend < 0 || friend >= MAX_CLIENTS){
            printf("Wrong friend ID: %ld\n", friend);
            return;
        }
        // printf("%s, %d\n", item, friend);
        msg.friends[friend] = 1;
        item = strtok(NULL, " ");
    }
    msg.mtype = 3;
    msg.sender_pid = getpid();
    msg.sender_id = ID;
    msg.reciever_id = -1;
    msg.command_type = ADD;
    sprintf(msg.text, "a");

    if(mq_send(server_queue_d, (const char *) &msg, sizeof(message), 1) == -1) exitErrno("Error in sending add query");
}
void execDel(char* commandBuffer){
    message msg;
    for(int i = 0; i < MAX_CLIENTS; i++) msg.friends[i] = 0;
    size_t friend;
    char* item = strtok(commandBuffer, " ");
    while(item){
        if(sscanf(item, "%ld", &friend) == 0 || friend < 0 || friend >= MAX_CLIENTS){
            printf("Wrong friend ID: %ld\n", friend);
            return;
        }
        // printf("%s, %d\n", item, friend);
        msg.friends[friend] = 1;
        item = strtok(NULL, " ");
    }
    msg.mtype = 3;
    msg.sender_pid = getpid();
    msg.sender_id = ID;
    msg.reciever_id = -1;
    msg.command_type = DEL;
    sprintf(msg.text, "a");

    if(mq_send(server_queue_d, (const char *) &msg, sizeof(message), 1) == -1) exitErrno("Error in sending del query");
}
void exec2Friends(char* text){

    message msg;
    msg.mtype = 3;
    msg.sender_pid = getpid();
    msg.sender_id = ID;
    msg.reciever_id = -1;
    msg.command_type = TO_FRIENDS;
    sprintf(msg.text, text);

    if(mq_send(server_queue_d, (const char *) &msg, sizeof(message), 1) == -1) exitErrno("Error in sending 2friends query");
}

void execFile(char* path);

void parseCommand(char *commandBuffer){

    if(strncmp("echo ", commandBuffer, 5) == 0)
        execEcho(&(commandBuffer[5]));   
    else if (strncmp("list", commandBuffer, 4) == 0)
    {
        execList();
    }else if (strncmp("2one ", commandBuffer, 5) == 0)
    {
        char* brk = strchr(&(commandBuffer[5]), ' ');
        *brk = '\n';
        int recID;
        if(sscanf(commandBuffer+5, "%d", &recID) == 0){
            printf("Wrong receiver ID\n");
            return; 
        }
        // printf("text: %s id: %d\n", brk+1, recID);
        exec2One(brk+1, recID);

    }else if (strncmp("2all ", commandBuffer, 5) == 0)
    {
        exec2All(&(commandBuffer[5]));
    }else if (strncmp("2friends ", commandBuffer, 9) == 0)
    {
        exec2Friends(&(commandBuffer[9]));
    }else if (strncmp("friends ", commandBuffer, 8) == 0)
    {
        execFriends(&(commandBuffer[8])); 
    }else if (strncmp("add ", commandBuffer, 4) == 0)
    {
        execAdd(&(commandBuffer[4])); 
    }else if (strncmp("del ", commandBuffer, 4) == 0)
    {
        execDel(&(commandBuffer[4])); 
    }else if(strncmp("friends", commandBuffer, 7) == 0)
    {
        execFriends("");
    }else if(strncmp("stop", commandBuffer, 4) == 0){
        raise(SIGINT);
    }else if(strncmp("file ", commandBuffer, 5) == 0){
        execFile(&(commandBuffer[5]));
    }
}

void execFile(char* path){
    char* c = path;
    while (*c != 0 ) {if(*c == 10) *c = 0; c++;}
    FILE* fp = fopen(path, "r");
    if(fp == NULL) {
        perror("File not found");
        return;
    }
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, fp)) != -1) {
       parseCommand(line);
    }

    fclose(fp);
    if (line)
        free(line);
}

int main(int argc, char** argv){
    if(argc != 2) exitError("Bad number of args");

    signal(SIGUSR1, responseHandler);

    struct sigaction sig;
    sigemptyset(&sig.sa_mask);
    sigaddset(&sig.sa_mask, SIGINT);
    sig.sa_flags = 0;
    sig.sa_handler = sigintHandler;
    sigaction(SIGINT, &sig, NULL);


    // signal(SIGINT, sigintHandler);
    init(argv[1]);

        //// command prompt
    char commandBuffer[256];
    printf("\n");
    while(1){
        printf(">");
        fgets(commandBuffer, 256, stdin);
        parseCommand(commandBuffer); 
    }        
}