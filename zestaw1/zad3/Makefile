CC=gcc
FLAGS= -Wall -Werror -pedantic
LFLAGS= -L ./

all: main_static main_shared main_dynamic

static: zad3.o 
	$(CC) $(FLAGS)

zad3.o: zad3.c
	$(CC) $(FLAGS) -c $?

main_static: zad3.o libmylib.a
	$(CC) $(FLAGS) $^ $(LFLAGS) -o $@ 

main_shared: LFLAGS += -Wl,-rpath=./
main_shared: zad3.o libmylib.so
	$(CC) $(FLAGS) $^ $(LFLAGS) -lmylib -o $@

main_dynamic: LFLAGS += -Wl,-rpath=./ -ldl -D DLL
main_dynamic: zad3.c libmylib.so
	$(CC) $(FLAGS) $^ $(LFLAGS) -o $@

libmylib.a: mylib_static.o
	ar rcs $@ $?

libmylib.so: FLAGS += -shared
libmylib.so: mylib_shared.o
	$(CC) $(FLAGS) $? -o $@

mylib_shared.o: FLAGS += -fPIC
mylib_shared.o mylib_static.o: mylib.c
	$(CC) $(FLAGS) -c $? -o $@

clean:
	rm -f *.o *.a *.so main_shared main_dynamic main_static
