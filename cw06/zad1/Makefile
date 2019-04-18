

CC=gcc

FLAGS= -Wall

all: server client

client: client.c 
	$(CC) $(FLAGS) -o $@ $?

server: server.c
	$(CC) $(FLAGS) -o $@ $?
	
clean:
	rm -rf server client


