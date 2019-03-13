CC=gcc
CFLAGS=-Wall -I../zad1/Include -c
LDFLAGS=-L../zad1
OPTIM=0
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

# Different optimizations for the program are to be tested.
# It is not specified that the library should also be compiled
# with different optimizations, so it's not done
program.o : $(SOURCES) ../zad1/Include
	$(CC) $(CFLAGS) -O$(OPTIM) $(SOURCES) -o $@

program_dynamic.o : $(SOURCES)
	$(CC) $(CFLAGS) -O$(OPTIM) $(SOURCES) -DDYNAMIC_LOAD -o $@

# assumption is made that the required libraries are ready
# and only targets for program are written
# (as would be the case in life, where required libraries
# are usually installed in binary form instead of from source)

test : shared static dynamic
	./test.sh

clean :
	-rm program{,_}{,static,shared,dynamic}{,.o} tmpfile* logfile_{shared,static,dynamic}

.PHONY : all clean shared static dynamic test
