#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/time.h>

void exitError(char* msg){
    printf(msg);
    exit(1);
}

void exitErrno(char* msg){
    perror(msg);
    exit(1);
}



struct timeval curr_time(){
    struct timeval t;
    gettimeofday(&t, NULL);
    return t;
}

long int time_diff(struct timeval t1, struct timeval t2){
    return (t2.tv_usec - t1.tv_usec + 1000000) % 1000000;
}


void print_time(struct timeval t){
    printf("%ld%ld", t.tv_sec, t.tv_usec);
}

void print_curr_time(void){
    print_time(curr_time());
}

int secureAtoi(char* txt){
    int ret = atoi(txt);
    if(ret > 0) return ret;
    exitErrno("Error while converting to int");
    return -1;
}