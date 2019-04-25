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

#define SERVER_KEY "/server"

void handleInit(message *msg);
void handleEcho(message *msg);
void handleList(message *msg);
void handle2one(message *msg);
void handle2all(message *msg);
void handle2friends(message *msg);
void handleAddDel(message *msg);
void handleStop(message *msg);
void handle_msg(message *msg);
void handleFriends(message *msg);

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
    
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i].active == 0) continue;
         
        message response;
        response.mtype = 1;
        response.sender_pid = getpid();
        response.sender_id = i;
        sprintf(response.text, "INIT");
        response.command_type = STOP;
        if(mq_send(clients[i].queue_id, (const char *) &response, sizeof(message), 1) == -1){
            perror("Error in sending id to client");
        }     
        kill(clients[i].pid, SIGUSR1); 

    }
    message rcvMsg;
    while(clients_count > 0){
        
        if(mq_receive(serverQueueD, (char *) &rcvMsg, MAX_MESSAGE_SIZE, NULL) == -1) exitErrno("Receiving message failed");
        handle_msg(&rcvMsg); 

    }  
    mq_close(serverQueueD);
    mq_unlink(SERVER_KEY);
    
    if(serverQueueD != -1){
        if(msgctl(serverQueueD, IPC_RMID, NULL) == -1){
            printf("Error in deleting queue\n");
            return;
        }
        printf("Queue deleted successfuly\n");
    }
    _exit(0);
}

void handle_msg(message *msg){
    switch ( msg->command_type)
    {
        case INIT:
                handleInit(msg);
            break;
    
        case ECHO:
                handleEcho(msg);
            break;
        case LIST:
                handleList(msg);
            break;
        case TO_ONE:
                handle2one(msg);
            break;
        case TO_ALL:
                handle2all(msg);
            break;
        case TO_FRIENDS:
                handle2friends(msg);
            break;
        case FRIENDS:
                handleFriends(msg);
            break;
        case ADD:
        case DEL:
                handleAddDel(msg);
            break;
        case STOP:
                handleStop(msg);
            break;
        default:
        printf("Unknown command\n");
        return;
            break;
    }
}

void handleStop(message *msg){
    printf("Handling stop\n");
    clients[msg->sender_id].active = 0;
    for(int i = 0; i < MAX_CLIENTS; i++) clients[msg->sender_id].friends[i] = 0;
    mq_close(clients[msg->sender_id].queue_id);
    clients_count--;
}

void handleInit(message *msg){
    printf("Handling INIT\n");
    if(clients_count < MAX_CLIENTS){

        size_t index = 0;
        while(index < MAX_CLIENTS){
            if(clients[index].active == 0) break;
            index++;
        }

        clients[index].pid = msg->sender_pid;
        clients[index].queue_id = mq_open(msg->text, O_WRONLY); 
        clients[index].active = 1;
        if(clients[index].queue_id == -1) exitErrno("Error in opening client queue");



        message response;
        response.mtype = 1;
        response.sender_pid = getpid();
        response.sender_id = index;
        sprintf(response.text, "INIT");
        response.command_type = INIT;
        if(mq_send(clients[index].queue_id, (const char *) &response, sizeof(message), 1) == -1){
            exitErrno("Error in sending id to client");
        }        
        printf("send to client new ID  = %ld\n", index);
        clients_count++;

    }
    else printf("server client list full\n");
}

void handleEcho(message *msg){
    printf("Handling echo\n");
    char date[128];
    char buff[MAX_MSG_LEN];
    FILE* datef = popen("date", "r");
    fgets(date, 128, datef);
    pclose(datef);
    printf("DATE: %s\n", date);
    strtok(date, "\n");
    message response;
    response.mtype = 1;
    response.sender_id = msg->sender_id;
    response.sender_pid = getpid();
    response.command_type = PRINT;
    snprintf(buff, MAX_MSG_LEN, "\"%s\": %s", date, msg->text);
    strcpy(response.text, buff); 


    printf("Sending response\n");
    if(mq_send(clients[msg->sender_id].queue_id, (const char *) &response, sizeof(message), 1) == -1){
        exitErrno("Error in sending message");
    }
    printf("SENDING SIGUSR1 to %d\n", msg->sender_pid);
    const union sigval sig;
    
    sigqueue(msg->sender_pid, SIGUSR1, sig);
}


void handleList(message *msg){
    printf("Handling list\n");
    char buf[900];
    char elem[20];
    buf[0] = 0;
    size_t off =0;
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(clients[i].active == 1){
            snprintf(elem, 20, "[%d:%d] ", i, clients[i].pid);
            memcpy(buf+off, elem, strlen(elem));
            off += strlen(elem);
        }
    }

    message response;
    response.mtype = 1;
    response.sender_id = msg->sender_id;
    response.sender_pid = getpid();
    response.command_type = PRINT;
    strcpy(response.text, buf);

    printf("Sending response\n");
    if(mq_send(clients[msg->sender_id].queue_id, (const char *) &response, sizeof(message), 1) == -1){
        exitErrno("Error in sending message");
    }
    printf("SENDING SIGUSR1 to %d\n", msg->sender_pid);
    const union sigval sig;
    
    sigqueue(msg->sender_pid, SIGUSR1, sig);
}

void handle2one(message *msg){
    printf("Handling twoOne\n");
    if(msg->reciever_id >= MAX_CLIENTS || clients[msg->reciever_id].active == 0){
        strcpy(msg->text, "Client with given id does not exists");
        handleEcho(msg);
        return;
    }
    client current_client = clients[msg->reciever_id];
    char date[128];
    char buff[MAX_MSG_LEN-30];
    FILE* datef = popen("date", "r");
    fgets(date, 128, datef);
    pclose(datef);
    printf("DATE: %s\n", date);
    strtok(date, "\n");
    
    msg->mtype = 1;
    msg->command_type = PRINT;
    snprintf(buff, MAX_MSG_LEN, "\"%s\": %s", date, msg->text);
    strcpy(msg->text, buff);

    printf("Sending response\n");
    if(mq_send(current_client.queue_id, (const char *) msg, sizeof(message), 1) == -1){
        exitErrno("Error in sending message");
    }
    printf("SENDING SIGUSR1 to %d\n", current_client.pid);
    const union sigval sig;
    
    sigqueue(current_client.pid, SIGUSR1, sig);
}

void handle2all(message *msg){

    printf("Handling twoAll\n");
    char date[128];
    char buff[MAX_MSG_LEN-30];
    FILE* datef = popen("date", "r");
    fgets(date, 128, datef);
    pclose(datef);
    printf("DATE: %s\n", date);
    strtok(date, "\n");
    
    msg->mtype = 1;
    msg->command_type = PRINT;
    snprintf(buff, MAX_MSG_LEN, "\"%s\": %s", date, msg->text);
    strcpy(msg->text, buff);

    printf("Sending response\n");
    const union sigval sig;

    for(int i = 0; i < MAX_CLIENTS; i++){
        if(i != msg->sender_id && clients[i].active == 1){
            if(mq_send(clients[i].queue_id, (const char *) msg, sizeof(message), 1) == -1)  exitErrno("Error in sending message");
            
            sigqueue(clients[i].pid, SIGUSR1, sig);
        }
    }
}

void handle2friends(message *msg){

    printf("Handling twoFriends\n");
    char date[128];
    char buff[MAX_MSG_LEN-30];
    FILE* datef = popen("date", "r");
    fgets(date, 128, datef);
    pclose(datef);
    printf("DATE: %s\n", date);
    strtok(date, "\n");
    
    msg->mtype = 1;
    msg->command_type = PRINT;
    snprintf(buff, MAX_MSG_LEN, "\"%s\": %s", date, msg->text);
    strcpy(msg->text, buff);

    printf("Sending response\n");
    const union sigval sig;
    client *currClient = &(clients[msg->sender_id]);
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(i != msg->sender_id && clients[i].active == 1 && currClient->friends[i]){
            if(mq_send(clients[i].queue_id, (const char *) msg, sizeof(message), 1) == -1)  exitErrno("Error in sending message");
            sigqueue(clients[i].pid, SIGUSR1, sig);
        }
    }
}

void handleFriends(message *msg){

    printf("Handling Friends\n");
    for(int i = 0; i < MAX_CLIENTS; i++){
        clients[msg->sender_id].friends[i] = msg->friends[i];
        printf("%d, ", msg->friends[i]);
    }
    printf("\n");
}

void handleAddDel(message *msg){

    printf("Handling Friends\n");
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        if(msg->friends[i])
            clients[msg->sender_id].friends[i] = msg->command_type == ADD ? 1 : 0;
        printf("%d, ", clients[msg->sender_id].friends[i]);
    }
    printf("\n");
}


int main(){
    struct sigaction sig;
    sigemptyset(&sig.sa_mask);
    sigaddset(&sig.sa_mask, SIGINT);
    sig.sa_flags = 0;
    sig.sa_handler = deleteQueue;
    sigaction(SIGINT, &sig, NULL);
    if(atexit(deleteQueue)) exitError("Can't initialize atexit");


    printf("%s\n", SERVER_KEY);

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MESSAGE_SIZE;
    attr.mq_curmsgs = 0;

    serverQueueD = mq_open(SERVER_KEY, O_RDONLY | O_CREAT, 0660, &attr);
    if(serverQueueD == -1)
        exitErrno("Error in creating public server queue\n");

    for(int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].active = 0;
        for(int j = 0; j < MAX_CLIENTS; j++){
            clients[i].friends[j] = 0;
        }
    }

    message rcvMsg;


    while(1){
        if(mq_receive(serverQueueD, (char *) &rcvMsg, MAX_MESSAGE_SIZE, NULL) == -1) exitErrno("Receiving message failed");

       handle_msg(&rcvMsg); 

    }
    



    

    // struct msgbuf msg;

    // msg.mtext[0] = 'a';
    // msg.mtype = 0;

    // msgsnd(msgq, &msg, sizeof(struct msgbuf) - sizeof(long), );
}