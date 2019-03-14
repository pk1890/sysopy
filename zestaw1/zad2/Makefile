CC=gcc
FLAGS= -Wall -Werror -pedantic -std=c11
LFLAGS= -L $(LIBDIR)
LIBDIR= ../zad1/

all: zad2_static zad2_shared
libs: static shared


zad2.o: zad2.c
	$(CC) $(FLAGS) $? -c -o $@

zad2_static: zad2.o $(LIBDIR)libmylib.a 
	$(CC) $(FLAGS) $^ -o $@

zad2_shared: LFLAGS += -Wl,-rpath=./
zad2_shared: $(LIBDIR)libmylib.so zad2.o
	$(CC) $(FLAGS) $^ $(LFLAGS) -lmylib -o $@
	
clean:
	rm -f *.so *.out *.o *.a
