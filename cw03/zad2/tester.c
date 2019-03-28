#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define TIME_FMT "%Y.%m.%d %H:%M:%S"

void exitError(char* err){
    printf(err);
    exit(1);
}
int main(int argc, char *argv[]) {
	srand(time(NULL));

	if (argc != 5) exitError("Params are: file name, pmin, pmax, bytes\n");

	char *file_name = argv[1];
	int pmin = atoi(argv[2]);
	int pmax = atoi(argv[3]);
	int bytes = atoi(argv[4]);

	char *string = malloc(bytes+2);
    if(string == NULL) exitError("Error in allocating string\n");
	for (int i = 0; i < bytes; i++) string[i] = 'X';
	string[bytes] = '\0';

	char *date = malloc(30);
    if(string == NULL) exitError("Error in allocating date \n ");

	while (1) {
		int wait = rand() % (abs(pmax - pmin) + 1) + pmin;
		sleep(wait);

		FILE *file = fopen(file_name, "a");

		time_t t = time(NULL);

		strftime(date, 30, TIME_FMT, gmtime(&t));

		fprintf(file, "%d %s %s\n", wait, date, string);
        printf("CHANGE! %s %s ", date, file_name);
		fclose(file);
	}
}
