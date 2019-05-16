
#ifndef _COMMON_H
#define _COMMON_H

#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>

#define PROJECT_ID 1
#define BUFF 0
#define EMPTY_SPACES 1
#define ON_BELT 2

void exitError(char* msg);
void exitErrno(char* msg);

struct timeval curr_time();
long int time_diff(struct timeval t1, struct timeval t2);
void print_time(struct timeval t);
void print_curr_time(void);

typedef struct{
    int mass;
    pid_t loader;
    struct timeval loaded_at;
} package;

package createPackage(int mass);


typedef struct{
    int capacity;
    int max_capacity;
} truck;

typedef struct{
    int first;
    int last;
    int size;
    int isEmpty;
    int maxWeight;
    int weight;
    int semaphores;
    int isTruckerDone;
    int childFinished;
    package buff[];
} belt;

int secureAtoi(char* txt);
belt* getBelt();
belt* createBelt(int size, int maxWeight);
void deleteBelt(belt* b);

void getPackageFromBelt(belt *b, truck *t);
void putPkgOnBelt(belt* b, package pkg);


#endif