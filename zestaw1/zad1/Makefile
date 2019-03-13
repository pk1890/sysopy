CC=gcc
FLAGS= -Wall -Werror -pedantic -std=c11
LFLAGS= -L ./

all: test_static test_shared
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

test.o: test.c
	$(CC) $(FLAGS) $? -c -o $@

test_static: test.o libmylib.a 
	$(CC) $(FLAGS) $^ -o $@

test_shared: LFLAGS += -Wl,-rpath=./
test_shared: libmylib.so test.o
	$(CC) $(FLAGS) $^ $(LFLAGS) -lmylib -o $@
	
clean:
	rm -f *.so *.out *.o *.a
