
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

int server_queue_d;
int ID = -1;

int client_queue_d;

void exitError(char* msg){
    printf("%s\n", msg);
    exit(1);
}

void exitErrno(char* msg){
    perror(msg);
    exit(1);
}

void responseHandler(){
    message msg;
    if(msgrcv(client_queue_d, &msg, MSG_SIZE, 0, 0) < 0) exitErrno("Receiving message failed");
    switch (msg.command_type)
    {
        // case INIT:
        //         ID = msg.sender_id;
        //         printf("RECEIVED ID FROM SERVER - %d\n", ID);
        //     break;
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
    sprintf(msg.text, "");

    if(msgsnd(server_queue_d, &msg, MSG_SIZE, 0) == -1) exitErrno("Error in sending init query");
    deleteQueue();
}

void deleteQueue(){
    if(client_queue_d != -1){
        if(msgctl(client_queue_d , IPC_RMID, NULL) == -1){
            printf("Error in deleting queue\n");
            return;
        }
        printf("Queue deleted successfuly\n");
    }
    _exit(0);
}
void init(int serverKey){
    server_queue_d = msgget(serverKey, 0);
    if(server_queue_d == -1) exitError("Error in openning server queue");


    signal(SIGINT, deleteQueue);
    if(atexit(deleteQueue)) exitError("Can't initialize atexit");
    char* path = getenv("HOME");
    if(path == NULL)
        exitError("Error in reading env var");
    key_t key = ftok(path, getpid());

    printf("%d, PID: %d\n", key, getpid());

    client_queue_d = msgget(key, IPC_CREAT | IPC_EXCL | 0666);
    if(client_queue_d == -1) exitErrno("Error in creating client queue");

    message msg;
    msg.mtype = 1;
    msg.sender_pid = getpid();
    msg.sender_id = -1;
    msg.command_type = INIT;
    sprintf(msg.text, "%d", key);

    if(msgsnd(server_queue_d, &msg, MSG_SIZE, 0) == -1) exitErrno("Error in sending init query");

    printf("QUEUE key sent to server\n");
    if(msgrcv(client_queue_d, &msg, MSG_SIZE, 0, 0) < 0) exitErrno("Receiving message failed");
    ID = msg.sender_id;
    printf("RECEIVED ID FROM SERVER - %d\n", ID);
    // if(msgrcv(client_queue_d, &msg, MSG_SIZE, 0, 0) < 0) exitErrno("Receiving message failed");
    // printf("Message received: \n");
    // printf("%s\n", msg.text);

}

void execEcho(char* text){

    message msg;
    msg.mtype = 1;
    msg.sender_pid = getpid();
    msg.sender_id = ID;
    msg.command_type = ECHO;
    sprintf(msg.text, text);

    if(msgsnd(server_queue_d, &msg, MSG_SIZE, 0) == -1) exitErrno("Error in sending init query");
}

void execList(){

    message msg;
    msg.mtype = 1;
    msg.sender_pid = getpid();
    msg.sender_id = ID;
    msg.command_type = LIST;
    sprintf(msg.text, "");

    if(msgsnd(server_queue_d, &msg, MSG_SIZE, 0) == -1) exitErrno("Error in sending init query");
}

void exec2One(char* text, int recID){

    message msg;
    msg.mtype = 1;
    msg.sender_pid = getpid();
    msg.sender_id = ID;
    msg.reciever_id = recID;
    msg.command_type = TO_ONE;
    sprintf(msg.text, text);

    if(msgsnd(server_queue_d, &msg, MSG_SIZE, 0) == -1) exitErrno("Error in sending init query");
}
void exec2All(char* text){

    message msg;
    msg.mtype = 1;
    msg.sender_pid = getpid();
    msg.sender_id = ID;
    msg.reciever_id = -1;
    msg.command_type = TO_ALL;
    sprintf(msg.text, text);

    if(msgsnd(server_queue_d, &msg, MSG_SIZE, 0) == -1) exitErrno("Error in sending init query");
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
        printf("%s, %d\n", item, friend);
        msg.friends[friend] = 1;
        item = strtok(NULL, " ");
    }
    msg.mtype = 1;
    msg.sender_pid = getpid();
    msg.sender_id = ID;
    msg.reciever_id = -1;
    msg.command_type = FRIENDS;
    sprintf(msg.text, "");

    if(msgsnd(server_queue_d, &msg, MSG_SIZE, 0) == -1) exitErrno("Error in sending init query");
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
        printf("%s, %d\n", item, friend);
        msg.friends[friend] = 1;
        item = strtok(NULL, " ");
    }
    msg.mtype = 1;
    msg.sender_pid = getpid();
    msg.sender_id = ID;
    msg.reciever_id = -1;
    msg.command_type = ADD;
    sprintf(msg.text, "");

    if(msgsnd(server_queue_d, &msg, MSG_SIZE, 0) == -1) exitErrno("Error in sending init query");
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
        printf("%s, %d\n", item, friend);
        msg.friends[friend] = 1;
        item = strtok(NULL, " ");
    }
    msg.mtype = 1;
    msg.sender_pid = getpid();
    msg.sender_id = ID;
    msg.reciever_id = -1;
    msg.command_type = DEL;
    sprintf(msg.text, "");

    if(msgsnd(server_queue_d, &msg, MSG_SIZE, 0) == -1) exitErrno("Error in sending init query");
}
void exec2Friends(char* text){

    message msg;
    msg.mtype = 1;
    msg.sender_pid = getpid();
    msg.sender_id = ID;
    msg.reciever_id = -1;
    msg.command_type = TO_FRIENDS;
    sprintf(msg.text, text);

    if(msgsnd(server_queue_d, &msg, MSG_SIZE, 0) == -1) exitErrno("Error in sending init query");
}

int main(int argc, char** argv){
    if(argc != 2) exitError("Bad number of args");

    signal(SIGUSR1, responseHandler);
    signal(SIGINT, sigintHandler);
    int serverKey = atoi(argv[1]);
    init(serverKey);
    while(1){

        //// command prompt
    char commandBuffer[256];
    printf("\n");
    while(1){
        printf(">");
        fgets(commandBuffer, 256, stdin);
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
                continue;
            }
            printf("text: %s id: %d\n", brk+1, recID);
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
        }
        
    }        



        
    }
}