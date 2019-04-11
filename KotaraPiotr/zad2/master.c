#include <fcntl.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_LEN 256

void exitError(char* msg){
    printf("%s\n", msg);
    exit(1);
}

int main(int argc, char** argv){ 

    if(argc != 2) printf("Usage: master <fifo_name>\n");

	if (mkfifo(argv[1], S_IWUSR | S_IRUSR) < 0)
		exitError("Error in creating FIFO");

    int fifo = open(argv[1], O_RDONLY);
    char* buff= calloc(MAX_LEN, sizeof(char));

    while(read(fifo, buff, MAX_LEN) != 0){
        printf("%s\n", buff);
    }
    
	
    close(fifo);


    free(buff);    

}