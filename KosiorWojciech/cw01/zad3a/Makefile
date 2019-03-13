CC=gcc
CFLAGS=-Wall -I../zad1/Include -c
LDFLAGS=-L../zad1

SOURCES=../zad2/program.c

all : shared static dynamic

shared : program_shared

static : program_static

dynamic : program_dynamic

program_dynamic : program_dynamic.o 
	$(CC) program_dynamic.o $(LDFLAGS) -ldl -o $@

program_static : program.o 
	$(CC) program.o $(LDFLAGS) -static -lsysopyfind -o $@

program_shared : program.o 
	$(CC) program.o $(LDFLAGS) -lsysopyfind -o $@

program.o : ../zad2/program.c ../zad1/Include
	$(CC) $(CFLAGS) $(SOURCES) -o $@

program_dynamic.o : ../zad2/program.c
	$(CC) $(CFLAGS) $(SOURCES) -DDYNAMIC_LOAD -o $@

# assumption is made that the required libraries are ready
# and only targets for program are specified
# (as would be the case in life, where required libraries
# are usually installed in binary form instead of from source)

test : shared static dynamic
	./test.sh

clean :
	-rm program{,_}{,static,shared,dynamic}{,.o} tmpfile* logfile*

.PHONY : all clean shared static dynamic test
