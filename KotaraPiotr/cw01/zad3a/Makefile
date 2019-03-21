CC=gcc
FLAGS= -Wall -Werror 
LFLAGS= -L $(LIBDIR)
LIBDIR= ../zad1/

all: main_static main_shared main_dynamic

zad3.o: zad3.c
	$(CC) $(FLAGS) -c $? -o $@

main_static: zad3.o $(LIBDIR)libmylib.a
	$(CC) $(FLAGS) $^ $(LFLAGS) -o $@ 

main_shared: LFLAGS += -Wl,-rpath=./
main_shared: zad3.o $(LIBDIR)libmylib.so
	$(CC) $(FLAGS) $^ $(LFLAGS) -lmylib -o $@

main_dynamic: LFLAGS += -Wl,-rpath=./ -ldl -D DYNAMIC
main_dynamic: zad3.c $(LIBDIR)libmylib.so
	$(CC) $(FLAGS) $^ $(LFLAGS) -o $@

clean:
	rm -f *.o *.a *.so main_shared main_dynamic main_static
