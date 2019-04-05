CC=gcc
CFLAGS = -Wall



all: catcher sender
sender: sender.c setSignals.o
	$(CC) $(CFLAGS) -o $@ $?

catcher: catcher.c setSignals.o
	$(CC) $(CFLAGS) -o $@ $?

setSignals.o: setSignals.c
	$(CC) $(CFLAGS) -c -o $@ $?

clean:
	rm -f sender *.o catcher