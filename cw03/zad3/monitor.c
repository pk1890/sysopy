
#define _XOPEN_SOURCE  700
#include<stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <libgen.h>
#include <time.h>

#define TIME_FMT "_%Y-%m-%d_%H-%M-%S"


volatile int monitoring_time = 30;

void exitError(char* err){
    printf(err);
    exit(1);
}

void monitor_file(char* path, int sec_no,rlim_t cpu,rlim_t mem);
void monitor_file_cp(char* path, int sec_no, rlim_t cpu,rlim_t mem);

int main(int argc, char** argv)
{
    if(argc != 6) exitError("Bad number of args!\n");
    monitoring_time = atoi(argv[2]);
    rlim_t cpu = atoi(argv[4]);
	rlim_t mem = atoi(argv[5]) *1024*1024;

    FILE * listaf;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    char* token;
    char* path_to_monitored;
    int sec_no;
    struct rusage r_usage, r_usage_next;

    listaf = fopen(argv[1], "r");
    if (listaf == NULL)
        exit(EXIT_FAILURE);

    struct stat st = {0};
    if (stat("./archiwum", &st) == -1) {
        mkdir("./archiwum", 0700);
    }
    path_to_monitored = calloc(sizeof(char), 512);
    while ((read = getline(&line, &len, listaf)) != -1) {
        //czytaj wejscie
        token = strtok(line, " ");
        strcpy(path_to_monitored, token);
        if(!(token = strtok(NULL, " "))) exitError("Bad arguments \n");
        sec_no = atoi(token);
        token = strtok(NULL, " ");
        if((token = strtok(NULL, " "))) exitError("Bad arguments \n");
        ///
        printf("=================Token: %s, sec: %d\n", path_to_monitored, sec_no);
        if(strcmp(argv[3], "cp") == 0)
            monitor_file_cp(path_to_monitored, sec_no, cpu, mem);
        else
            monitor_file(path_to_monitored, sec_no, cpu, mem);
    }
    fclose(listaf);
    int status;
    if (getrusage(RUSAGE_CHILDREN, &r_usage)) exitError("Blad odczytu rusage\n");
    pid_t pid = wait(&status);
    while (pid != -1){
        if(WIFEXITED(status))
            printf("Proces %d utworzyl %d kopii pliku \n", pid, WEXITSTATUS(status));
        else printf("Proces nie zakonczyl sie prawidlowo, liczba kopii nieznana");
        
        if (getrusage(RUSAGE_CHILDREN, &r_usage_next)) exitError("Blad odczytu rusage\n");

        printf("  user: %ld.%08lds\n",
                r_usage_next.ru_utime.tv_sec - r_usage.ru_utime.tv_sec,
                r_usage_next.ru_utime.tv_usec - r_usage.ru_utime.tv_usec);
        printf("  sys:  %ld.%08lds\n",
                r_usage_next.ru_stime.tv_sec - r_usage.ru_stime.tv_sec,
                r_usage_next.ru_stime.tv_usec - r_usage.ru_stime.tv_usec);
        printf("Memory: %ld\n", r_usage_next.ru_maxrss);

        if (getrusage(RUSAGE_CHILDREN, &r_usage)) exitError("Blad odczytu rusage\n");

        pid = wait(&status);
    }


    if (line)
        free(line);
    free(path_to_monitored);
    exit(EXIT_SUCCESS);
}

void monitor_file_cp(char* path, int sec_no, rlim_t cpu, rlim_t mem){


    pid_t pid = fork();
    int i = 0;
    short times_modified = 0;    
    struct stat statt;
    time_t last_modified = 0;
    char *buff;
    long file_len;
    char *path_to_arch;
    pid_t cp_pid;

    if(pid == 0){
        printf("%s, %d , czekam....\n", path, sec_no);   
        //system("sleep 3");

        lstat(path, &statt);
        printf("Inicjalizacja monitoringu, wczytywanie do pamięci... Czas monitoringu co %d \n",  monitoring_time);
        last_modified = statt.st_mtime;
        cp_pid = fork();
        if(cp_pid == 0){
            char* mTimetxt = calloc(sizeof(char), 64); 
            strftime(mTimetxt, 64, TIME_FMT, localtime(&last_modified)); 
            path_to_arch = calloc(sizeof(char), 256); 
            snprintf(path_to_arch, 256 ,"./archiwum/%s%s", basename(path), mTimetxt); 
            printf("cp %s %s\n", path, path_to_arch);
            free(mTimetxt);
            execlp("cp", "cp", path, path_to_arch, NULL); 
        }else if(cp_pid < 0) exitError("Error in making process"); 
        else{ 
            wait(NULL); 
        } 


        while(i < monitoring_time)
        {
            lstat(path, &statt);
            if(statt.st_mtime > last_modified){
                last_modified = statt.st_mtime;

                cp_pid = fork();
                if(cp_pid == 0){
                    char* mTimetxt = calloc(sizeof(char), 64); 
                    strftime(mTimetxt, 64, TIME_FMT, localtime(&last_modified)); 
                    path_to_arch = calloc(sizeof(char), 256); 
                    snprintf(path_to_arch, 256 ,"./archiwum/%s%s", basename(path), mTimetxt); 
                    printf("cp %s %s\n", path, path_to_arch);
                    printf("Zmiana! Tworze plik %s \n", path_to_arch);
                    free(mTimetxt);
                    execlp("cp", "cp", path, path_to_arch, NULL); 
                }else if(cp_pid < 0) exitError("Error in making process"); 
                else{ 
                    wait(NULL); 
                } 
                    times_modified++;

            }
            sleep(sec_no);
            i+=sec_no;
        }

        free(buff);
        exit(times_modified);

    }
    
    else if (pid < 0) exitError("Error in forking\n");
    else{
        return;
    }



}

void monitor_file(char* path, int sec_no, rlim_t cpu, rlim_t mem){
    pid_t pid = fork();
    int i = 0;
    short times_modified = 0;    
    struct stat statt;
    time_t last_modified = 0;
    char *buff;
    long file_len;
    char *path_to_arch;

    if(pid == 0){
        printf("%s, %d , czekam....\n", path, sec_no);   
        //system("sleep 3");

        lstat(path, &statt);
        printf("Inicjalizacja monitoringu, wczytywanie do pamięci... Czas monitoringu co %d \n",  monitoring_time);
        last_modified = statt.st_mtime;
        FILE* f = fopen(path, "r");
        if(!f) exitError("Error in opening the file\n");
        fseek(f, 0L, SEEK_END);
        file_len = ftell(f);
        rewind(f);

        buff = (char*)calloc(file_len, sizeof(char));
        if(fread(buff, sizeof(char), file_len, f) != file_len) exitError("Error in loading a file");
        fclose(f);

        while(i < monitoring_time)
        {
            lstat(path, &statt);
            if(statt.st_mtime > last_modified){
                printf("%d, %d", i, monitoring_time);
                last_modified = statt.st_mtime;
                FILE* f = fopen(path, "r");
                if(!f) exitError("Error in opening the file\n");
                fseek(f, 0L, SEEK_END);
                file_len = ftell(f);
                rewind(f);
                
                free(buff);
                buff = (char*)calloc(file_len, sizeof(char));
                if(fread(buff, sizeof(char), file_len, f) != file_len) exitError("Error in loading a file");
                fclose(f);

                char* mTimetxt = calloc(sizeof(char), 64);
                strftime(mTimetxt, 64, TIME_FMT, localtime(&last_modified));
                path_to_arch = calloc(sizeof(char), 256);
                snprintf(path_to_arch, 256 ,"./archiwum/%s%s", basename(path), mTimetxt);
                printf("Zmiana! Tworze plik %s \n", path_to_arch);

                f = fopen(path_to_arch, "w+");
                fwrite(buff, sizeof(char), file_len, f);

                fclose(f);

                free(mTimetxt);
                free(path_to_arch);
                times_modified++;

            }
            sleep(sec_no);
            i+=sec_no;
        }

        free(buff);
        exit(times_modified);

    }
    
    else if (pid < 0) exitError("Error in forking\n");
    else{
        return;
    }




}