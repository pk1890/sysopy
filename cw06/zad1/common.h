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

#define MAX_MSG_LEN 1024
#define MAX_CLIENTS 20
#define PROJECT_ID 0x01

typedef struct{
    pid_t pid;
    int queue_id;
}client;


typedef struct{
    long mtype;
    pid_t sender_pid;
    short command_type;
    char text[MAX_MSG_LEN];
} message;
const size_t MSG_SIZE = sizeof(message) - sizeof(long);
#endif