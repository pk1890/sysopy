CC=gcc
CFLAGS=-Wall -I../zad1/Include -c
LDFLAGS=-L../zad1 -static -lsysopyfind -o program

all : program

program : program.o library
	$(CC) program.o $(LDFLAGS)

program.o : program.c ../zad1/Include
	$(CC) $(CFLAGS) program.c

# this recursive make call results in program being relinked
# on every make all even if nothing changed
library :
	$(MAKE) -C ../zad1 static

test : program
	./test.sh

clean :
	-rm program program.o tmpfile* logfile

.PHONY : all clean library test
