CC=gcc
FLAGS= -Wall -Werror -pedantic -std=c11
LFLAGS= -L ./

all: zad2_static zad2_shared
libs: static shared


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

zad2.o: zad2.c
	$(CC) $(FLAGS) $? -c -o $@

zad2_static: zad2.o libmylib.a 
	$(CC) $(FLAGS) $^ -o $@

zad2_shared: LFLAGS += -Wl,-rpath=./
zad2_shared: libmylib.so zad2.o
	$(CC) $(FLAGS) $^ $(LFLAGS) -lmylib -o $@
	
clean:
	rm -f *.so *.out *.o *.a
