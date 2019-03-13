CC=gcc
CFLAGS=-Wall -I./Include

all : static shared

static : libsysopyfind.a

shared : libsysopyfind.so

sysopyfind.o : sysopyfind.c Include/sysopyfind.h
	$(CC) $(CFLAGS) sysopyfind.c -c 

libsysopyfind.a : sysopyfind.o
	ar rcs libsysopyfind.a sysopyfind.o

libsysopyfind.so : sysopyfind.c Include/sysopyfind.h
	$(CC) $(CFLAGS) sysopyfind.c -fPIC -shared \
	-Wl,-soname,libsysopyfind.so -o libsysopyfind.so

clean :
	-rm *.o *.a *.so

.PHONY : all static dynamic clean
