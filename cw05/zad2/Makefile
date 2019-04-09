
CC=gcc

FLAGS= -Wall

all: master slave

master: master.c 
	$(CC) $(FLAGS) -o $@ $?

slave: slave.c
	$(CC) $(FLAGS) -o $@ $?
	
clean:
	rm -rf master slave fif
