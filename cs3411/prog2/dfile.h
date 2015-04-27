#ifndef DFILE_H
#define DFILE_H

#include <sys/types.h>

#define MIN_SIZE 36
#define SIZE_MUL 4

typedef struct {
	int size;
	off_t next,prev,free;
	void* data;
	int fd;
} DFile;

DFile* dopen(char*, int);
off_t dalloc(DFile*, int);
int dfree(DFile*, off_t);
int dclose(DFile*);

#endif