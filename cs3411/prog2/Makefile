CC=gcc
CFLAGS=-std=c99 -g

libDFile.a	: dopen.o dclose.o dalloc.o dfree.o
		rm -f libDFile.a
		ar -cq libDFile.a dopen.o dclose.o dalloc.o dfree.o

dopen.o		: dfile.h dopen.c
		$(CC) $(CFLAGS) -c dopen.c

dclose.o	: dfile.h dclose.c
		$(CC) $(CFLAGS) -c dclose.c

dalloc.o	: dfile.h dalloc.c
		$(CC) $(CFLAGS) -c dalloc.c

dfree.o		: dfile.h dfree.c
		$(CC) $(CFLAGS) -c dfree.c

test5		: libDFile.a
		$(CC) $(CFLAGS) test5.c libDFile.a -o test5

clean:
		rm -f libDFile.a dopen.o dclose.o dalloc.o dfree.o