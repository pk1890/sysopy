
#include <sys/msg.h> 
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <signal.h>
#include "common.h"
#include <errno.h>


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

void init(int serverKey){
    server_queue_d = msgget(serverKey, 0);
    if(server_queue_d == -1) exitError("Error in openning server queue");



    message msg;
    msg.mtype = 1;
    msg.sender_pid = getpid();
    msg.sender_id = -1;
    msg.command_type = INIT;
    sprintf(msg.text, "%d", )

    if(msgsnd(server_queue_d, &msg, MSG_SIZE, 0) == -1) exitErrno("Error in sending init query");

    if(msgrcv(server_queue_d, &msg, MSG_SIZE, 0, 0) < 0) exitErrno("Receiving message failed");
    ID = msg.sender_id;
    printf("RECEIVED ID FROM SERVER - %d\n", ID);
}

int main(int argc, char** argv){
    if(argc != 2) exitError("Bad number of args");
    int serverKey = atoi(argv[1]);
    init(serverKey);
}