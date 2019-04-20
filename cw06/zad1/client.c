
#include <sys/msg.h> 
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <signal.h>
#include "common.h"
#include <errno.h>
#include <unistd.h>

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

void deleteQueue(){
    if(client_queue_d != -1){
        if(msgctl(client_queue_d , IPC_RMID, NULL) == -1){
            printf("Error in deleting queue\n");
            return;
        }
        printf("Queue deleted successfuly\n");
    }
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

    printf("%d\n", key);

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
    msg.mtype = 1;
    msg.sender_pid = getpid();
    msg.sender_id = ID;
    msg.command_type = ECHO;
    sprintf(msg.text, "DUUUPPPPAAAA\n");

    if(msgsnd(server_queue_d, &msg, MSG_SIZE, 0) == -1) exitErrno("Error in sending init query");
    if(msgrcv(client_queue_d, &msg, MSG_SIZE, 0, 0) < 0) exitErrno("Receiving message failed");
    printf("Message received: \n");
    printf("%s\n", msg.text);

}

int main(int argc, char** argv){
    if(argc != 2) exitError("Bad number of args");
    int serverKey = atoi(argv[1]);
    init(serverKey);
}