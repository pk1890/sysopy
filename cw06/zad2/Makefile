

CC=gcc

FLAGS= -Wall 

all: server client

client: client.c 
	$(CC) $(FLAGS) -o $@ $? -lrt

server: server.c
	$(CC) $(FLAGS) -o $@ $? -lrt
	
clean:
	rm -rf server client


