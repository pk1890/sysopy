CC=gcc
CFLAGS = -Wall



all: monitor tester

monitor: monitor.c
	$(CC) $(CFLAGS) -o $@ $?

tester: tester.c 
	$(CC) $(CFLAGS) -o $@ $?

clean:
	rm -rf archiwum monitor tester