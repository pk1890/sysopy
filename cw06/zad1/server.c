#include <sys/msg.h> 
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <signal.h>
#include "common.h"
#include <errno.h>

#define serverQ 0x01


client clients[MAX_CLIENTS];
pid_t clients_count = 0;

int serverQueueD = -1; 
void exitError(char* msg){
    printf("%s\n", msg);
    exit(1);
}

void exitErrno(char* msg){
    perror(msg);
    exit(1);
}

int getClientQueue(pid_t client_pid){
    for(int i = 0; i < clients_count; i++){
        if(clients[i].pid == client_pid) return clients[i].queue_id;
    }
    return -1;
}

void deleteQueue(){
    if(serverQueueD != -1){
        if(msgctl(serverQueueD, IPC_RMID, NULL) == -1){
            printf("Error in deleting queue\n");
            return;
        }
        printf("Queue deleted successfuly\n");
    }
}

void handleMsg(message *msg){
    switch ( msg->command_type)
    {
        case INIT:
                handleInit(msg);
            break;
    
        default:
            break;
    }
}


void handleInit(message *msg){
    if(clients_count < MAX_CLIENTS){
        clients[clients_count] = msg->sender_pid;
        int client_key;
        if(sscanf(msg->text, "%d", &client_key) < 0)
            exitError("Failed to read client key");
        clients[clients_count].pid = msg->sender_pid;
        clients[clients_count].queue_id = client_key;
        clients_count++;

    }
    else printf("server client list full\n");
}

void handleEcho(message *msg){

    char* date[128], buff[MAX_MSG_LEN];
    FILE* datef = popen("date", "r");
    fgets(date, 128, datef);
    pclose(datef);

    message response;
    response.mtype = 1;
    response.sender_pid = getpid();
    response.command_type = 0;
    snprintf(buff, MAX_MSG_LEN, "[%s]: %s", date, msg->text);

    if(msgsnd(getClientQueue(msg->sender_pid), &response, MSG_SIZE, 0)      
    kill(SIGUSR1, msg->sender_pid);
}

int main(){
    signal(SIGINT, deleteQueue);
    if(atexit(deleteQueue)) exitError("Can't initialize atexit");
    char* path = getenv("HOME");
    if(path == NULL)
        exitError("Error in reading env var");
    key_t key = ftok(path, PROJECT_ID);

    serverQueueD = msgget(key, IPC_CREAT | IPC_EXCL);
    if(serverQueueD == -1)
        exitError("Error in creating public server queue\n");

    message rcvMsg;

    while(1){
       if(msgrcv(serverQueueD, &rcvMsg, MAX_MSG_LEN, -7, 0) == -1) exitError("Error in reading from queue");

       handle_msg(rcvMsg); 

    }
    



    
    printf("%s", key);

    // struct msgbuf msg;

    // msg.mtext[0] = 'a';
    // msg.mtype = 0;

    // msgsnd(msgq, &msg, sizeof(struct msgbuf) - sizeof(long), );
}