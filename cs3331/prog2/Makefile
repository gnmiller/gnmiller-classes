CC=gcc
CFLAGS=-g

main	: main.c
	$(CC) $(CFLAGS) main.c -o main
	
qsort	: qsort.c
	$(CC) $(CFLAGS) qsort.c -o qsort
	
merge	: merge.c
	$(CC) $(CFLAGS) merge.c -o merge
	
clean	:
	rm main merge qsort; ./clear_shm

all	:
	make main qsort merge
	