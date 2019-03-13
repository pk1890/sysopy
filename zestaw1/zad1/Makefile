CC=gcc
FLAGS= -Wall -Werror -pedantic -std=c11
LFLAGS= -L ./

all: static shared

mylib_shared.o: FLAGS += -fPIC
mylib_static.o mylib_shared.o: mylib.c
	$(CC) $(FLAGS) -c $? -o $@

libmylib.a: mylib_static.o
	ar rcs $@ $?

libmylib.so: FLAGS += -shared
libmylib.so: mylib_shared.o
	$(CC) $(FLAGS) $? -o $@

static: libmylib.a

shared: libmylib.so

test.o: test.c
	$(CC) $(FLAGS) $? -c -o $@

test_static: libmylib.a test.o
	$(CC) $(FLAGS) $^ -o $@

test_shared: LFLAGS += -Wl,-rpath=./
test_shared: libmylib.so test.o
	$(CC) $(FLAGS) $< $(LFLAGS) -lmylib -o $@
	


simple:	
	gcc -Wall mylib.c test.c -o main

oldstatic:
	make clean
	gcc -Wall -c mylib.c
	ar rcs libmylib.a mylib.o
	gcc -Wall -c test.c
	gcc -Wall test.o libmylib.a -o test_static

oldshared:
	make clean
	gcc -c -Wall -Werror -fpic mylib.c
	gcc -shared -o libmylib.so mylib.o
	gcc -L ./ -Wall -o test_shared test.c -lmylib


clean:
	rm -f *.so *.out
