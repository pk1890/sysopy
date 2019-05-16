#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include"common.h"

key_t get_key()
{
    char* home = getenv("HOME");
    if(home == NULL) exitErrno("Error in reading env var");
    key_t key = ftok(home, PROJECT_ID);
    if (key == -1) exitErrno("Error in creating key");
    return key;
}
void incSem(belt *b, int semNo){
    struct sembuf sbuf;
    sbuf.sem_num = semNo;
    sbuf.sem_op = 1;
    sbuf.sem_flg = 0;
    while (semop(b->semaphores, &sbuf, 1) == -1) {
        if (errno != EINTR || b->isEmpty) exit(0);
    }
}
void decSem(belt *b, int semNo){
    struct sembuf sbuf;
    sbuf.sem_num = semNo;
    sbuf.sem_op = -1;
    sbuf.sem_flg = 0;
    while (semop(b->semaphores, &sbuf, 1) == -1) {
        if (errno != EINTR || b->isEmpty) exit(0);
    }
}

int decSemNonblock(belt *b, int sem_num)
{
    struct sembuf sops[1] = {{sem_num, -1, IPC_NOWAIT}};
    if (semop(b->semaphores, sops, 1) == -1) {
        if (errno == EAGAIN || (errno == EINTR && !b->isEmpty)) return -1;
        else exit(0);
    }
    return 0;
}

belt* createBelt(int size, int maxWeight){
    int shm_id = shmget(get_key(), sizeof(belt) + size * sizeof(package), IPC_CREAT | IPC_EXCL | 0666);
    if (shm_id == -1) exitErrno("Error in creating shared memory");
    belt *b = (belt*)shmat(shm_id, NULL, 0);
    if(b == NULL) exitErrno("Error in maping shared mem");
    int sem_id = semget(get_key(), 3, IPC_CREAT | IPC_EXCL | 0666);
    if (sem_id == -1) {
        perror("Error in semaphore");
        shmdt(b);
        shmctl(shm_id, IPC_RMID, NULL);
        exit(-1);
    }
    b->first = 0;
    b->last = 0;
    b->isTruckerDone = 0;
    b->isEmpty = 1;
    b->maxWeight = maxWeight;
    b->size = size;
    b->semaphores = sem_id;

    semctl(sem_id, BUFF, SETVAL, 1);
    semctl(sem_id, EMPTY_SPACES, SETVAL, size);
    semctl(sem_id, ON_BELT, SETVAL, 0);

    return b;
}

belt* getBelt(){
    int shm_id = shmget(get_key(), 0, 0);
    if (shm_id == -1) exitErrno("Error getting shmemory");
    belt *b = shmat(shm_id, NULL, 0);
    if (b == (void*) -1) exitErrno("Error mapping semaphore");
    return b;
}

void deleteBelt(belt* b){
    if(b){
        semctl(b->semaphores, 0, IPC_RMID);
        shmctl(shmget(get_key(), 0, 0), IPC_RMID, NULL);
        b = NULL;
    }
}

package createPackage(int mass){
    package pkg;
    pkg.loader = getpid();
    pkg.mass =mass;

    return pkg;
}

void putPkgOnBelt(belt* b, package pkg){
    int size, weight, maxWeight, taken;
    if(b->isTruckerDone) exit(0);
    int done = 0;
    printf("Waiting for empty place in belt\n");
    struct timeval start_time = curr_time();

    while(!done){
        
        
        decSem(b, EMPTY_SPACES);
        decSem(b, BUFF);
        gettimeofday(&(pkg.loaded_at),NULL);
        if(b->weight + pkg.mass <= b->maxWeight && !b->isTruckerDone){

            b->buff[b->last] = pkg;
            b->last = (b->last + 1) % b->size;
            b->weight += pkg.mass;

            size = b->size;
            taken = (b->last + size - 1 - b->first) % size + 1;

            weight = b->weight;
            maxWeight = b->maxWeight;
            done = 1;


            incSem(b, ON_BELT);
        } else{
            incSem(b, EMPTY_SPACES);
            incSem(b, BUFF);
            continue;
        }
        incSem(b, BUFF);
    }
    print_time(pkg.loaded_at);
    printf(", PID:%d, %dkg package loaded after %ld microsec, belt status: %d/%d %dkg/%dkg\n",  pkg.loader, pkg.mass, time_diff(start_time, curr_time()), taken, size, weight, maxWeight);

}

void getPackageFromBelt(belt *b, truck *t){

    int size, weight, maxWeight, taken;
    if(b->isTruckerDone) exit(0);

    struct timeval start_time = curr_time();
    printf("Waiting for package in belt\n");
    
    decSem(b, ON_BELT);
    decSem(b, BUFF);

    package pkg = b->buff[b->first];

    if(pkg.mass <= t->capacity){

        b->first = (b->first + 1) % b->size;
        b->weight -= pkg.mass;
        t->capacity -= pkg.mass;

        size = b->size;
        taken = (b->last + size - b->first) % size;

        weight = b->weight;
        maxWeight = b->maxWeight;



        incSem(b, EMPTY_SPACES);
    } else{
        incSem(b, ON_BELT);
        print_curr_time();
        printf(", unloading truck\n");
        print_curr_time();
        printf(", new truck arrived\n");
        t->capacity = t->max_capacity;
        incSem(b, BUFF);
        return;
    }
    incSem(b, BUFF);


    print_time(pkg.loaded_at);
    printf(", PID:%d, %dkg package loaded after %ld microsec, belt status: %d/%d %dkg/%dkg, Truck free space: %d/%dkg\n",  pkg.loader, pkg.mass, time_diff(start_time, curr_time()), taken, size, weight, maxWeight, t->capacity, t->max_capacity);

}
