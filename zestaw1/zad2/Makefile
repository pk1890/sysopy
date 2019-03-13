all: static shared

simple:	#simply compiles all files together
	gcc -Wall mylib.c test.c -o main

static:	#first compiles .c files to .o files, then compiles it together with main file
	make clean
	gcc -c -Wall zad2.c 
	gcc -Wall zad2.o ../zad1/libmylib.a -o program_static

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
