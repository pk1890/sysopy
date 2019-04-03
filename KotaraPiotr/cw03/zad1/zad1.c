#define _XOPEN_SOURCE 500
#include <dirent.h>
#include <ftw.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define TIME_FMT "%d.%m.%Y-%H:%M:%S"
#define nextArg() if(++currArg == argc) exitError("too few args")

void exitError(char* err);
int cmpTime(time_t modTime, char* option, time_t date);
void treeByDate(char* path, char* option, time_t date);
time_t parseDate(char* date);
void displayFile(const char* path, struct stat statt);
int cmpTime(time_t modTime, char* option, time_t date);
int treeNftw (const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf);
void lsSearch(const char* path);

char* globalOp;
time_t globalTime;

int main(int argc, char** argv){
    if(argc != 5) exitError("Too few args\n");
    if(strcmp(argv[4], "nftw") != 0){

    treeByDate(argv[1], argv[2], parseDate(argv[3]));
    } else{
        globalOp = argv[2];
        globalTime = parseDate(argv[3]);
        nftw(argv[1], treeNftw, 20, FTW_PHYS);

    } 
}

void treeByDate(char* path, char* option, time_t date){
    DIR* dir;
    struct dirent* dent;
    struct stat statt;
    dir = opendir(path);
    if(dir == NULL) return;
    if(cmpTime(statt.st_mtime, option, date)){
        lsSearch(path);
        wait(NULL);
    }
    dent = readdir(dir);
    while (dent){
        if(strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0){
            dent = readdir(dir);
            continue;
        }

        char* rec_path = malloc(strlen(path) + strlen(dent->d_name) + 10);
        if(!rec_path) exitError("Mem allocation error");
        sprintf(rec_path, "%s/%s", path, dent->d_name);

        if(lstat(rec_path, &statt) == -1) exitError("Error in listing file props");

        //if(cmpTime(statt.st_mtime, option, date)){
            //displayFile(rec_path, statt);
      //  }

        if(S_ISDIR(statt.st_mode)){
            treeByDate(rec_path, option, date);
        }
        free(rec_path);   
        dent = readdir(dir);

    }
    // if(-1 != closedir(dir)) exitError("Error in closing dir");
    
}

void lsSearch(const char* path){
   pid_t pid = fork();

   if(pid == 0){
       printf("=========================================================Process ID = %d\n", getpid());
       execlp("ls","ls", "-l", path, NULL);
   } else if(pid < 0){
       exitError("Error in forking");
   } else{
       return;
   }
}


int treeNftw (const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    if(ftwbuf->level == 0) return 0;
    if(!cmpTime(sb->st_mtime, globalOp, globalTime)) return 0;
     lsSearch(fpath);
     wait(NULL);
    //displayFile(fpath, *sb);
    return 0;
}

time_t parseDate(char* date){
    struct tm time;
    char* res = strptime(date, TIME_FMT, &time);
    if (res == NULL || *res != '\0') {
		exitError("Incorrect date format");
	}
    time_t t = mktime(&time);
    if(t == -1) exitError("Wrong data format");
    return t;
}

void displayFile(const char* path, struct stat statt){
    char* type;
    if(S_ISREG(statt.st_mode)){
        type = "Regular file";
    } else if (S_ISDIR(statt.st_mode))
    {
        type = "Directory";
    }else if (S_ISCHR(statt.st_mode))
    {
        type = "Character device";
    }else if (S_ISBLK(statt.st_mode))
    {
        type = "Block device";
    }else if (S_ISFIFO(statt.st_mode))
    {
        type = "FIFO";
    }else if (S_ISLNK(statt.st_mode))
    {
        type = "Symbolic link";
    }else if (S_ISSOCK(statt.st_mode))
    {
        type = "Socket";
    }else{
        type = "Unknown type";
    }
    
    long size = statt.st_size;
    time_t accessTime = statt.st_atime;
    time_t modTime = statt.st_mtime;
    char* aTimetxt = calloc(sizeof(char), 64);
    char* mTimetxt = calloc(sizeof(char), 64);
    strftime(aTimetxt, 64, TIME_FMT, localtime(&accessTime)); 
    strftime(mTimetxt, 64, TIME_FMT, localtime(&modTime));
    printf("%s \t \t type: %s\t\t size: %ld\t\t access time: %s \t\t mod time: %s\n", path, type,  size, aTimetxt, mTimetxt); 
}

int cmpTime(time_t modTime, char* option, time_t date){
    if(strcmp(option, "=") == 0) {
		return modTime == date;
	} else if (strcmp(option, ">") == 0) {
		return modTime > date;
	} else if (strcmp(option, "<") == 0) {
		return modTime < date;
	} else {
		exitError("Wrong operand");
	}
	return 0;
}

void exitError(char* err){
    printf(err);
    exit(1);
}
