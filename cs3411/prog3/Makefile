CC=gcc
CFLAGS=-std=c99 -g

link	: link.c
	$(CC) $(CFLAGS) link.c -o link

ring	: ring.c
	$(CC) $(CFLAGS) ring.c -o ring

test	: test.c
	$(CC) $(CFLAGS) test.c

all	: link ring

clean	:
	rm a.out ring link
