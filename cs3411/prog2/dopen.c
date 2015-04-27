#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "dfile.h"
#include <string.h>
/**
 * Open a new file.
 *
 * @param name The file name of the file.
 * @param initSize The number of bytes in the file.
 *                 Must be at least 36 bytes and a multiple of 4.
 *
 * @return a pointer to a DFile structure, or NULL if there are any problems.
 *         "errno" is set to EINVAL if the size is invalid.
 */
DFile* dopen(char* name, int initSize) {
	
	if ( MIN_SIZE < 36 || initSize % SIZE_MUL != 0) {
		errno = EINVAL;
		return NULL;
	} // error
	errno = 0;
	// set up the file and variables
	DFile* ret;
	// space for all our pointers
	ret = (DFile*) malloc( sizeof(DFile) ) ;
	
	if( ret <= 0 ){
		errno = EINVAL;
		return NULL;
	} // error
	
	// open the file
	ret->fd = open( name, O_RDWR|O_CREAT, S_IRWXU ); // ReadWrite, Create if no exist, perms: 0700
	
	// initSize = size of first node + ALL overhead
	// overhead = 2x size prev next and free (4,4,8,8,8) eg min file = 32 + 1 byte of data
	int tsize;
	tsize = initSize - sizeof(off_t)*3; // next,prev,free
	tsize = tsize - sizeof(int)*2; // 2 sizes
	ret->size = tsize;
	
	if( (ret->size) <= 0 ) return NULL; // error
	if ( (ret->fd) <= 0 ) return NULL; // error

	ret->next = (off_t)0;
	ret->prev = (off_t)0;
	ret->free = sizeof(off_t); //free node is first node
	
	if ( ret->data < 0 ) return NULL; // error
	
	// write the node into the file
	lseek(ret->fd,0,SEEK_SET);
	if( write(ret->fd, &ret->free, sizeof(off_t) ) <= 0 ) return NULL;
	if( write(ret->fd, &ret->prev, sizeof(off_t) ) <= 0 ) return NULL;
	if( write(ret->fd, &ret->size, sizeof(int) ) <= 0 ) return NULL;
	if( lseek(ret->fd, ret->size, SEEK_CUR) <= 0 ) return NULL;
	// seek forward (whats in data isnt really important yet
	// if we wanted null data we could loop and set or call calloc instead
	//if( write(ret->fd, ret->data, ret->size) ) { perror("Error"); return NULL; }
	if( write(ret->fd, &ret->size, sizeof(int) ) <= 0 ) return NULL;
	if( write(ret->fd, &ret->next, sizeof(off_t)) <= 0 ) return NULL;
	return ret;
}