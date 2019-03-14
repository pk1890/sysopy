#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include <zconf.h>


#ifndef DYNAMIC
#include "../zad1/mylib.h"
#endif


#ifdef DYNAMIC
#include <dlfcn.h>

void * mylib;

typedef struct {
    size_t index;
    struct Free_qnode* next;
    
} Free_qnode;

typedef struct {
    struct Free_qnode* first;
    struct Free_qnode* last;
    struct Free_qnode* ward;
} Free_queue;

typedef struct
{
    char **data;
    size_t block_no;
    struct Free_queue* free_indeces;
    size_t first_free;
} block_arr;

block_arr* (*init_array)(size_t);
size_t (*find)(block_arr*, char*, char*, char*);
int (*write_to_tmp)(char* , char* , char*);
size_t (*read_file_to_block)(block_arr* , const char*);
int (*delete_block)(block_arr* , size_t);
void (*delete_block_arr)(block_arr** );


void dynamic_load_error() {
	fprintf(stderr, "Could not load dynamic library");
	exit(1);
}
#endif


typedef struct tms tms;
typedef struct {
	clock_t real_start;
	struct tms start;
	double elapsed_real;
	double elapsed_user;
	double elapsed_kernel;
    double elapsed_Cuser;
	double elapsed_Ckernel;
} timer;
void time_reset(timer* timers, int count) {
	for(int i = 0; i < count; i++) {
		timers[i].elapsed_real = 0;
		timers[i].elapsed_user = 0;
		timers[i].elapsed_kernel = 0;
		timers[i].elapsed_Cuser = 0;
		timers[i].elapsed_Ckernel = 0;

	}
}

double timediff(clock_t t1, clock_t t2) {
	return (double) (t2-t1) / sysconf(_SC_CLK_TCK);
}

void time_update(timer* t) {
	struct tms end;
	clock_t real_end = times(&end);

	t->elapsed_real += timediff(t->real_start, real_end);
	t->elapsed_user += timediff(t->start.tms_utime, end.tms_utime);
	t->elapsed_kernel  += timediff(t->start.tms_stime, end.tms_stime);
    t->elapsed_Cuser += timediff(t->start.tms_cutime, end.tms_cutime);
	t->elapsed_Ckernel  += timediff(t->start.tms_cstime, end.tms_cstime);
}

void time_start(timer *t){
    t->real_start = times(&(t->start));
}

void time_print(timer* t, char* label) {
    printf("================ %s", label);
    putchar('\n');
    printf("  real time:    %0.2fs", t->elapsed_real);
    printf("  user time:    %0.2fs", t->elapsed_user);
    printf("  kernel time:   %0.2fs", t->elapsed_kernel);
    printf("  child user time:    %0.2fs", t->elapsed_Cuser);
    printf("  child kernel time:   %0.2fs\n", t->elapsed_Ckernel);
}

void printHelp(){
    printf("Usage:\t<table_size> [command]...\n" \
		"Command can be one of:\n" \
		"\tsearch_directory <dir> <file> <tmp_file_name>\t- search for <file> in <dir>\n" \
		"\tdelete_block <index>\t- delete result from <index>\n" );
}


int main(int argc, char** argv){

    #ifdef DYNAMIC
	mylib = dlopen("../zad1/libmylib.so", RTLD_LAZY);
	if (mylib == NULL) dynamic_load_error();
	
    init_array = dlsym(mylib, "init_array");
    delete_block_arr = dlsym(mylib, "delete_block_arr");


	if (init_array == NULL || delete_block_arr == NULL) dynamic_load_error();
	#endif


    timer timers[4];
	time_reset(timers, 4);
	timer* find_timer  = &timers[0];
	timer* alloc_timer = &timers[1];
	timer* delete_timer   = &timers[2];
	timer* add_and_delete_timer   = &timers[3];

    if(argc <= 2) {
        printf("Bad number of arguments\n");
        printHelp();
        exit(EXIT_FAILURE);
    }

    block_arr* memory = init_array((size_t)atoi(argv[1]));

    for(size_t i = 2; i < argc; ++i)
    {
        if (strcmp(argv[i], "search_directory") == 0){
            if(argc-i <=3){
                printf("Bad number of arguments\n");
                exit(EXIT_FAILURE);
            }

            #ifdef DYNAMIC
            write_to_tmp = dlsym(mylib, "write_to_tmp");
            read_file_to_block = dlsym(mylib, "read_file_to_block");

            if(write_to_tmp == NULL || read_file_to_block == NULL){
                dynamic_load_error();
            }

            #endif

            char* dir = argv[++i];
            char* file = argv[++i];
            char* tmp = argv[++i];
            time_start(find_timer);
            //system("sleep 30");
            write_to_tmp(tmp, dir, file);
            time_update(find_timer);

            time_start(alloc_timer);
            read_file_to_block(memory, tmp);
            time_update(alloc_timer);
            
            //find(memory, dir, file, tmp);

        } else if (strcmp(argv[i], "delete_block") == 0){
            if(argc-i <=1){
                printf("Bad number of arguments\n");
                exit(EXIT_FAILURE);
            }

            #ifdef DYNAMIC
            delete_block = dlsym(mylib, "delete_block");

            if(write_to_tmp == NULL || read_file_to_block == NULL || delete_block == NULL){
                dynamic_load_error();
            }

            #endif
            

            size_t index = atoi (argv[++i]);
            time_start(delete_timer);
            delete_block(memory, index);
            time_update(delete_timer);
        }else if(strcmp(argv[i], "stress_test") == 0){
            if(argc-i <=4){
                printf("Bad number of arguments\n");
                exit(EXIT_FAILURE);
            }

            #ifdef DYNAMIC
            write_to_tmp = dlsym(mylib, "write_to_tmp");
            read_file_to_block = dlsym(mylib, "read_file_to_block");
            delete_block = dlsym(mylib, "delete_block");

            if(write_to_tmp == NULL || read_file_to_block == NULL || delete_block == NULL){
                dynamic_load_error();
            }

            #endif
            
            char* dir = argv[++i];
            char* file = argv[++i];
            char* tmp = argv[++i];
            size_t count = (size_t)atoi(argv[++i]);
            size_t index;

            write_to_tmp(tmp, dir, file);

            time_start(add_and_delete_timer);
            for(size_t i = 0; i < count; i++){
                index = read_file_to_block(memory, file);
                delete_block(memory, index);
            }
            time_update(add_and_delete_timer);
        }
        
        else{
            printf("Bad arguments\n");
            printHelp();
            exit(EXIT_FAILURE);
        }
        
    }


        time_print(find_timer, "Find time");
        time_print(alloc_timer, "Allocation time");
        time_print(delete_timer, "Deleting time");
        time_print(add_and_delete_timer, "Add and delete many times time");
}