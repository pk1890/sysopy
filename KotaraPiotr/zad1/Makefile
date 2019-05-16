

CC=gcc

FLAGS= -Wall 

all: loader trucker

trucker: trucker.c common.c belt.c
	$(CC) $(FLAGS) -o $@ $?

loader: loader.c common.c belt.c
	$(CC) $(FLAGS) -o $@ $?
	
clean:
	rm -rf loader trucker


