
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

//children vars
int stopped = 0;
short times_modified = 0;
typedef struct{
    char monitored_file[512];
    pid_t pid;
    int stopped;
    int interval;   
}child;

void printChild(child c){
    char* status = c.stopped == 0 ? "RUNNING" : "STOPPED";
    printf("======\nPath: %s\nPID: %d\nStatus: %s\nInterval: %d\n===========-\n", c.monitored_file, c.pid, status, c.interval);
}
void listAll(child* children, size_t n){
    for(size_t i = 0; i < n; i++)
    {
        if(children[i].stopped == 0)
            printf("[%d: %s]\t", children[i].pid, children[i].monitored_file);
        else
            printf("\033[0;31m [%d: %s]\t\033[0m", children[i].pid, children[i].monitored_file);

    }
    printf("\n");
    
}

void stop(pid_t pid, child* children, size_t n){
    for(size_t i = 0; i < n; i++)
    {
        if(children[i].pid == pid){
            if(children[i].stopped == 0){
                kill(pid, SIGUSR1);
                children[i].stopped = 1;
                return;
            }else{
                printf("Child already stopped\n");
                return;
            }
        }
    }
    printf("Wrong PID");
    
    
}
void stopAll(child* children, size_t n){

    for(size_t i = 0; i < n; i++){
        if(children[i].stopped == 0){
            kill(children[i].pid, SIGUSR1);
            children[i].stopped = 1;

        }
    }
}

void start(pid_t pid, child* children, size_t n){
    for(size_t i = 0; i < n; i++)
    {
        if(children[i].pid == pid){
            if(children[i].stopped == 1){
                kill(pid, SIGUSR1);
                children[i].stopped = 0;
                return;
            }else{
                printf("Child not stopped\n");
                return;
            }
        }
    }
    printf("Wrong PID");
    
    
}
void startAll(child* children, size_t n){

    for(size_t i = 0; i < n; i++){
        if(children[i].stopped == 1){
            kill(children[i].pid, SIGUSR1);
            children[i].stopped = 0;

        }
    }
}

void endAll(child* children, size_t n){

    for(size_t i = 0; i < n; i++){
        kill(children[i].pid, SIGUSR2);
    }
}

void exitError(char* err){
    printf(err);
    exit(1);
}

pid_t monitor_file(char* path, int sec_no);
pid_t monitor_file_cp(char* path, int sec_no);

int main(int argc, char** argv)
{
    if(argc != 4) exitError("Bad number of args!\n");

    FILE * listaf;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    size_t lines;
    char* token;
    char* path_to_monitored;
    int sec_no;
    child *children;


    listaf = fopen(argv[1], "r");
    if (listaf == NULL)
        exit(EXIT_FAILURE);
    

    char ch;
    lines = 0;
    while(!feof(listaf))
    {
        ch = fgetc(listaf);
        if(ch == '\n')
        {
            lines++;
        }
    }

    rewind(listaf);
    children = calloc(lines, sizeof(child));
    // printf("\n\n ILOSC LINII W LISTA: %ld", lines);

    

    struct stat st = {0};
    if (stat("./archiwum", &st) == -1) {
        mkdir("./archiwum", 0700);
    }
    size_t i = 0;
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

        strcpy(children[i].monitored_file, path_to_monitored);
        children[i].interval = sec_no;
        children[i].stopped = 0;
         



        if(strcmp(argv[2], "cp") == 0)
            children[i].pid = monitor_file_cp(path_to_monitored, sec_no);
        else
            children[i].pid = monitor_file(path_to_monitored, sec_no);

        printChild(children[i]);
        i++;
    }
    fclose(listaf);
    listAll(children, lines);

    //// command prompt
    sleep(1);
    char* commandBuffer = calloc(64, sizeof(char));
    printf("\n");
    while(1){
        printf(">");
        fgets(commandBuffer, 64, stdin);
        if(strncmp("list", commandBuffer, 4) == 0){
            listAll(children, lines);
        }
        else if(strncmp("stop all", commandBuffer, 8) == 0){
            stopAll(children, lines);
        }
        else if(strncmp("stop ", commandBuffer, 5) == 0){
            stop(atoi(commandBuffer+5), children, lines);
        }
        else if(strncmp("start all", commandBuffer, 9) == 0){
            startAll(children, lines);
        }
        else if(strncmp("start ", commandBuffer, 6) == 0){
            start(atoi(commandBuffer+5), children, lines);
        }
        else if(strncmp("end", commandBuffer, 3) == 0){
            endAll(children, lines);
            break;
        }
        
    }        




    int status;
    pid_t pid = wait(&status);
    while (pid != -1){
        if(WIFEXITED(status))
            printf("Proces %d utworzyl %d kopii pliku \n", pid, WEXITSTATUS(status));
        else printf("Proces nie zakonczyl sie prawidlowo, liczba kopii nieznana");
        pid = wait(&status);
    }    

    if (line)
        free(line);
    free(children);
    free(path_to_monitored);
    exit(EXIT_SUCCESS);
}
void sig_child_handler(int signo){
    if(signo == SIGUSR1){
        stopped = !stopped;
        if(stopped)
            printf("%d stopped\n", getpid());
        else 
            printf("%d started\n", getpid());
    }else if(signo == SIGUSR2){
        exit(times_modified);
    }
}
pid_t monitor_file_cp(char* path, int sec_no){


    pid_t pid = fork();

    if(pid == 0){

        struct sigaction act;
		act.sa_handler = sig_child_handler;
		sigemptyset(&act.sa_mask);
		act.sa_flags = 0;
		if (sigaction(SIGUSR1, &act, NULL) || sigaction(SIGUSR2, &act, NULL))
            exitError("Can't catch signals\n");

        struct stat statt;
        time_t last_modified = 0;
        char *buff = NULL;
        char *path_to_arch;
        pid_t cp_pid;
        printf("%s, %d , czekam....\n", path, sec_no);   
        //system("sleep 3");

        lstat(path, &statt);
        printf("Inicjalizacja monitoringu, wczytywanie do pamięci... \n");
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


        while(1)
        {
            if(stopped) continue;
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
        }
        if(buff != NULL)
            free(buff);
        exit(times_modified);
    }
    
    else if (pid < 0) exitError("Error in forking\n");
    else{
        return pid;
    }

    //program will not go there (hopefully)
    return -1;

}

pid_t monitor_file(char* path, int sec_no){
    pid_t pid = fork();
    struct stat statt;
    time_t last_modified = 0;
    char *buff;
    long file_len;
    char *path_to_arch;

    if(pid == 0){
        struct sigaction act;
		act.sa_handler = sig_child_handler;
		sigemptyset(&act.sa_mask);
		act.sa_flags = 0;
		if (sigaction(SIGUSR1, &act, NULL) || sigaction(SIGUSR2, &act, NULL))
            exitError("Can't catch signals\n");
        //system("sleep 3");

        printf("%s, %d , czekam....\n", path, sec_no);   
        lstat(path, &statt);
        printf("Inicjalizacja monitoringu, wczytywanie do pamięci... \n");
        last_modified = statt.st_mtime;
        FILE* f = fopen(path, "r");
        if(!f) exitError("Error in opening the file\n");
        fseek(f, 0L, SEEK_END);
        file_len = ftell(f);
        rewind(f);

        buff = (char*)calloc(file_len, sizeof(char));
        if(fread(buff, sizeof(char), file_len, f) != file_len) exitError("Error in loading a file");
        fclose(f);

        while(1)
        {
            if(stopped) continue;
            lstat(path, &statt);
            if(statt.st_mtime > last_modified){
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
        }

        free(buff);
        exit(times_modified);

    }
    
    else if (pid < 0) exitError("Error in forking\n");
    else{
        return pid;
    }



    //program will not go there (hopefully)
    return -1;

}