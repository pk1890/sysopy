#include<stdlib.h>
#include<stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/times.h> //do mierzenia czasu


#define nextArg() if(++currArg == argc) exitError("too few args")
#define CAR argv[currArg]

typedef struct tms tms;
typedef struct {
	clock_t real_start;
	struct tms start;
	double elapsed_real;
	double elapsed_user;
	double elapsed_kernel;
    double elapsed_Cuser;
	double elapsed_Ckernel;
    int used;
} timer;
void time_reset(timer* timers, int count) {
	for(int i = 0; i < count; i++) {
		timers[i].elapsed_real = 0;
		timers[i].elapsed_user = 0;
		timers[i].elapsed_kernel = 0;
		timers[i].elapsed_Cuser = 0;
		timers[i].elapsed_Ckernel = 0;
        timers[i].used = 0;

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
    t->used = 1;
}

void time_start(timer *t){
    t->real_start = times(&(t->start));
}

void time_print(timer* t, char* label) {
    if(t->used){
        printf("================ %s", label);
        putchar('\n');
        printf("  real time:    %0.2fs", t->elapsed_real);
        printf("  user time:    %0.2fs", t->elapsed_user);
        printf("  kernel time:   %0.2fs", t->elapsed_kernel);
        printf("  child user time:    %0.2fs", t->elapsed_Cuser);
        printf("  child kernel time:   %0.2fs\n", t->elapsed_Ckernel);
    }
}



void exitError(char* err);
void generate(char* pathName, size_t len, size_t blkSize);
void sortSys(char* pathName, size_t len, size_t blkSize);
void sortLib(char* pathName, size_t len, size_t blkSize);
void copySys(char* sourcePath, char* destPath, size_t len, size_t blkSize);
void copyLib(char* sourcePath, char* destPath, size_t len, size_t blkSize);

int main(int argc, char **argv){

    timer timers[4];
	time_reset(timers, 4);
	timer* copy_lib_t  = &timers[0];
	timer* copy_sys_t =  &timers[1];
	timer* sort_lib_t   = &timers[2];
	timer* sort_sys_t   = &timers[3];

    if(argc < 2) exitError("Too few args");
    for(size_t currArg =  1;  currArg < argc; ++currArg){
        if(strcmp(CAR, "gen") == 0){
            nextArg();
            char* path = CAR;
            nextArg();
            int len = atoi(CAR);
            nextArg();
            int blkSize = atoi(CAR);
            generate(path, len, blkSize);
        } else if (strcmp(CAR, "copyL") == 0)
        {
            nextArg();
            char* src = CAR;
            nextArg();
            char* dest = CAR;
            nextArg();
            int len = atoi(CAR);
            nextArg();
            int blkSize = atoi(CAR);
            time_start(copy_lib_t);
            copyLib(src, dest, len, blkSize);
            time_update(copy_lib_t); 
        } else if (strcmp(CAR, "copyS") == 0)
        {
            nextArg();
            char* src = CAR;
            nextArg();
            char* dest = CAR;
            nextArg();
            int len = atoi(CAR);
            nextArg();
            int blkSize = atoi(CAR);
            time_start(copy_sys_t);
            copySys(src, dest, len, blkSize); 
            time_update(copy_sys_t);
        }  else if (strcmp(CAR, "sortL") == 0)
        {
            nextArg();
            char* src = CAR;
            nextArg();
            int len = atoi(CAR);
            nextArg();
            int blkSize = atoi(CAR);
            time_start(sort_lib_t);
            sortLib(src, len, blkSize);
            time_update(sort_lib_t); 
        }else if (strcmp(CAR, "sortS") == 0)
        {
            nextArg();
            char* src = CAR;
            nextArg();
            int len = atoi(CAR);
            nextArg();
            int blkSize = atoi(CAR);
            time_start(sort_sys_t);
            sortSys(src, len, blkSize); 
            time_update(sort_sys_t);
        }
        else {
            exitError("wrong args");
        }
        
    }

    time_print(copy_lib_t, "Lib copy version: ");
    time_print(copy_sys_t, "Sys copy version: ");
    time_print(sort_lib_t, "Lib sort version: ");
    time_print(sort_sys_t, "Sys sort version: ");
    return 0;
}

void generate(char* pathName, size_t len, size_t blkSize){
    FILE* file = fopen(pathName, "w");
    if(!file) exitError("error opening file");
    FILE* urand = fopen("/dev/urandom", "r");
    if(!urand) exitError("Error opening urandom");

    for (int i = 0; i < len; i++)
		for (int j = 0; j < blkSize; j++)
			fputc(fgetc(urand), file);

    if(fclose(urand)) exitError("Error closing file");
    if(fclose(file)) exitError("Error closing file" );


}

void sortSys(char* pathName, size_t len, size_t blkSize){
    int file = open(pathName, O_RDWR);
    if(!file) exitError("error opening file");
    
    unsigned char* buff1 = calloc(blkSize, sizeof(unsigned char));
    if(!buff1) exitError("Error allocating mem");
    unsigned char* buff2 = calloc(blkSize, sizeof(unsigned char));
    if(!buff2) exitError("Error allocating mem");

    for(size_t i = 0; i < len-1; ++i){
        lseek(file, i*blkSize, SEEK_SET);
        if(read(file, buff1, blkSize*sizeof(unsigned char)) != blkSize) exitError("Error reading first buf");
        unsigned char minimum = buff1[0];
        size_t swap_index = i;
        for(size_t j = i+1; j < len; j++){
            lseek(file, j*blkSize, SEEK_SET);
            if(read(file, buff2, blkSize) != blkSize) exitError("Error reading sec buf");
                if(minimum > buff2[0])
                {
                    minimum = buff2[0];
                    swap_index = j;
                }
        }
    
        lseek(file, swap_index*blkSize, SEEK_SET);
        if(read(file, buff2, blkSize) != blkSize) exitError("Error reading buff2");

        lseek(file, i*blkSize, SEEK_SET);
        if(write(file, buff2, blkSize) != blkSize) exitError("Error writing buff2");
        lseek(file, swap_index*blkSize, SEEK_SET);
        if(write(file, buff1, blkSize) != blkSize) exitError("Error writing buff1");
    }

    free(buff1);
    free(buff2);

    if(close(file)) exitError("Error closing file");
}


void sortLib(char* pathName, size_t len, size_t blkSize){
    FILE* file = fopen(pathName, "r+");
    if(!file) exitError("error opening file");
    
    unsigned char* buff1 = calloc(blkSize, sizeof(unsigned char));
    if(!buff1) exitError("Error allocating mem");
    unsigned char* buff2 = calloc(blkSize, sizeof(unsigned char));
    if(!buff2) exitError("Error allocating mem");

    for(size_t i = 0; i < len-1; ++i){
        fseek(file, i*blkSize, SEEK_SET);
        if(fread(buff1, sizeof(unsigned char), blkSize, file) != blkSize) exitError("Error reading file");
        unsigned char minimum = buff1[0];
        size_t swap_index = i;
        for(size_t j = i+1; j < len; j++){
            fseek(file, j*blkSize, SEEK_SET);
            if(fread(buff2, sizeof(unsigned char), blkSize, file) != blkSize) exitError("Error reading file");
                if(minimum > buff2[0])
                {
                    minimum = buff2[0];
                    swap_index = j;
                }
        }
        
        fseek(file, swap_index*blkSize, SEEK_SET);
        if(fread(buff2, sizeof(unsigned char), blkSize, file) != blkSize) exitError("Error reading buff2");

        fseek(file, i*blkSize, SEEK_SET);
        if(fwrite(buff2, sizeof(unsigned char), blkSize, file) != blkSize) exitError("Error reading file");
        fseek(file, swap_index*blkSize, SEEK_SET);
        if(fwrite(buff1, sizeof(unsigned char), blkSize, file) != blkSize) exitError("Error reading file");
    }

    free(buff1);
    free(buff2);

    if(fclose(file)) exitError("Error closssssing file");
}

void copySys(char* sourcePath, char* destPath, size_t len, size_t blkSize){
    int src = open(sourcePath, O_RDONLY);
    if(!src) exitError("Error opening source file");
    int dest = open(destPath, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IROTH	);
    if(!dest) exitError("Error opening destination file");

    unsigned char* buff = calloc(blkSize, sizeof(unsigned char));
    if(!buff) exitError("Error allocating buffer");
    
    for(size_t i = 0; i < len; ++i){
        lseek(src, i*blkSize, SEEK_SET);

        if(read(src, buff, blkSize) != blkSize) exitError("error reading a source file");
        
        lseek(dest, i*blkSize, SEEK_SET);
        if(write(dest, buff, blkSize) != blkSize) exitError("Error in writing to destination file");

    }
    free(buff);
    if(close(src)) exitError("error closing source file");
    if(close(dest)) exitError("error closing destination file");
}

void copyLib(char* sourcePath, char* destPath, size_t len, size_t blkSize){
    FILE* src = fopen(sourcePath, "r");
    if(!src) exitError("Error opening source file");
    FILE* dest = fopen(destPath, "w");
    if(!dest) exitError("Error opening destination file");

    unsigned char* buff = calloc(blkSize, sizeof(unsigned char));
    if(!buff) exitError("Error allocating buffer");
    
    for(size_t i = 0; i < len; ++i){
        fseek(src, i*blkSize, SEEK_SET);
        fseek(dest, i*blkSize, SEEK_SET);

        if(fread(buff, sizeof(unsigned char), blkSize, src) != blkSize) exitError("error reading a source file");
        
        if(fwrite(buff, sizeof(unsigned char), blkSize, dest) != blkSize) exitError("Error in writing to destination file");

    }
    free(buff);
    if(fclose(src)) exitError("error closing source file");
    if(fclose(dest)) exitError("error closing destination file");
}
void exitError(char* err){
    printf(err);
    exit(1);
}