all: static shared

simple:	#simply compiles all files together
	gcc -Wall mylib.c test.c -o main

static:	#first compiles .c files to .o files, then compiles it together with main file
	make clean
	gcc -Wall -c mylib.c
	ar rcs libmylib.a mylib.o
	gcc -Wall -c test.c
	gcc -Wall test.o libmylib.a -o test_static

shared: #kompiluje jako biblioteke współdzieloną
	make clean
	gcc -c -Wall -Werror -fpic mylib.c
	gcc -shared -o libmylib.so mylib.o
	gcc -L ./ -Wall -o test_shared test.c -lmylib

clean:	#removes everything apart from source files; it's nice to have such cleaning command
	rm -f *.o main

reset:
	make clean
	rm -f *.so *.out
