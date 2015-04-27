CC=gcc
CFLAGS=-std=c99 -ggdb3
LDFLAGS=
.PHONY: all clean distclean

all: 

testpack: pack.o
	$(CC) $(CFLAGS) testpack.c -lm -o testpack pack.o

testunpack: unpack.o
	$(CC) $(CFLAGS) testunpack.c -lm -o testunpack unpack.o

unpack: unpack.c
	$(CC) $(CFLAGS) -c -std=c99 unpack.c -lm -o unpack.o

pack: pack.c
	$(CC) $(CFLAGS) -c -std=c99 pack.c -lm -o pack.o

clean:
	rm testpack
	rm testunpack
	rm pack.o
	rm unpack.o

distclean: clean