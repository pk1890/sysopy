#ifndef COMMON
#define COMMON

#include <sys/types.h>

#define INIT 0
#define ECHO 1
#define LIST 2
#define FRIENDS 3
#define TO_ALL 4
#define TO_FRIENDS 5
#define TO_ONE 6
#define STOP 7
#define ADD 8
#define DEL 9
#define PRINT 64

#define MAX_MSG_LEN 1024
#define MAX_CLIENTS 20
#define PROJECT_ID 0x01

typedef struct{
    pid_t pid;
    int queue_id;
    short active;
    short friends[MAX_CLIENTS];
}client;


typedef struct{
    long mtype;
    pid_t sender_pid;
    pid_t sender_id;
    short command_type;
    short reciever_id;
    char text[MAX_MSG_LEN];
    short friends[MAX_CLIENTS];
} message;
const size_t MSG_SIZE = sizeof(message) - sizeof(long);
#endif