
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>

#include"common.h"

sem_t *semaphores[3];


key_t get_key()
{
    char* home = getenv("HOME");
    if(home == NULL) exitErrno("Error in reading env var");
    key_t key = ftok(home, PROJECT_ID);
    if (key == -1) exitErrno("Error in creating key");
    return key;
}


void incSem(belt *b, int sem_num)
{
    while (sem_post(semaphores[sem_num]) == -1) {
        if (errno != EINTR || b->isEmpty) exit(0);
    }
}

void decSem(belt *b, int sem_num)
{
    while (sem_wait(semaphores[sem_num]) == -1) {
        if (errno != EINTR || b->isEmpty) exit(0);
    }
}


belt* createBelt(int size, int maxWeight){

    int shm_id = shm_open("/belt", O_RDWR | O_CREAT | O_EXCL, 0666);
    if (shm_id == -1) exitErrno("Error in opening semaphores");

    if (ftruncate(shm_id, sizeof(belt) + size * sizeof(package)) == -1) exitErrno("failed setting size of shared memory");

    belt *b = mmap(NULL, sizeof(belt) + size * sizeof(package), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);

    if (b == (void*) -1) exitErrno("Error in mapping shared mem");

    close(shm_id);

    b->first = 0;
    b->last = 0;
    b->size = size;
    b->weight = 0;
    b->maxWeight = maxWeight;
    b->isTruckerDone = 0;
    b->isEmpty = 0;
    b->childFinished = 0;

    if ((semaphores[BUFF] = sem_open("/buff", O_RDWR | O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) exitErrno("Error in openong semaphore");
    if ((semaphores[EMPTY_SPACES] = sem_open("/empty_spaces", O_RDWR | O_CREAT | O_EXCL, 0666, b->size)) == SEM_FAILED) exitErrno("Error in openong semaphore");
    if ((semaphores[ON_BELT] = sem_open("/on_belt", O_RDWR | O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED) exitErrno("Error in openong semaphore");
    return b;
}
belt* getBelt(){
    int shm_id = shm_open("/belt", O_RDWR, 0);
    if (shm_id == -1) exitError("Cant open sharedMem");

    int size;
    if (read(shm_id, &size, sizeof(int)) == -1) exitErrno("error in reading shared mem");

    belt *b = mmap(NULL, sizeof(belt) + size * sizeof(package), PROT_WRITE | PROT_READ, MAP_SHARED, shm_id, 0);
    if (b == (void*) -1) exitErrno("Error in mapping sharedmem");

    close(shm_id);

    if ((semaphores[BUFF] = sem_open("/buff", O_RDWR)) == SEM_FAILED) exitErrno("Error in openong semaphore");
    if ((semaphores[EMPTY_SPACES] = sem_open("/empty_spaces", O_RDWR)) == SEM_FAILED)exitErrno("Error in openong semaphore");
    if ((semaphores[ON_BELT] = sem_open("/on_belt", O_RDWR)) == SEM_FAILED) exitErrno("Error in openong semaphore");

    return b;
}

void closeSem(){
    
    sem_close(semaphores[BUFF]);
    sem_close(semaphores[EMPTY_SPACES]);
    sem_close(semaphores[ON_BELT]);
}

void deleteBelt(belt* b){
    sem_unlink("/buff");
    sem_unlink("/on_belt");
    sem_unlink("/empty_spaces");
    shm_unlink("/belt");
    exit(0);
}

package createPackage(int mass){
    package pkg;
    pkg.loader = getpid();
    pkg.mass =mass;

    return pkg;
}

void putPkgOnBelt(belt* b, package pkg){
    int size, weight, maxWeight, taken;
    if(b->isTruckerDone) closeSem(); 
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
    if(b->isTruckerDone) closeSem();

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
