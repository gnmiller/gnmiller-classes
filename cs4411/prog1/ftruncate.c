/*
 * Greg Miller	gnmiller
 *
 * Program 1
 * CS4411
 * Due: 1/19/2015
 * ftruncate.c
 * 	Wraps the system call ftruncate( fd, length )
 *		If the size of the file is less than length the the data after
 *			length is file is written with a pattern (all 0 in this case)
 *		If the size if greater or equal to length then truncate( ... ) is
 *			called as normal.
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#ifndef NOCALL
int ftruncate( int fd, off_t length ){
	struct stat f_info;
	
	#ifdef GNM_DEBUG
	printf("MODIFIED FTRUNCATE\n");
	#endif
	
	/* stat the file to get size of other info */
	errno = 0;
	if ( fstat( fd, &f_info ) != 0 ) {
		fprintf( stderr, "Failed to stat file specified by fd (%d): %s\n", fd, strerror(errno));
		return -1;
	}
	
	int cur_max = f_info.st_size;
	int write_size = cur_max - length;
	
	// open a new fd to guarantee we open with write access
	int new_fd = dup(fd);
	fcntl( new_fd, F_SETFL, O_WRONLY );
	
	errno = 0;
	if ( new_fd < 0 ){
		fprintf( stderr, "Failed to open file specified by fd (%d) for writing: %s\n", fd, strerror(errno));
		return -1;
	}
	/* first check the size of the file against the length
	 *
	 * if they  match or are greater we do not need to add functionality
	 * thus return the system call as normal
	 *
	 * otherwise we store the file offset
	 * seek to the end of new length and start writing
	 * as: 
		write( path_fd, 0, sizeof(uint_32))
	 */
	if( length == cur_max || length > cur_max ){
		return syscall( __NR_ftruncate, fd, length );		
	} else {
		int to_write[length];
		int store_pos;
		// create the data to write to 0 out the file
		memset( to_write, 0, length );
		
		// store the current file offset
		store_pos = lseek( new_fd, 0 , SEEK_CUR );
		if( store_pos < 0 ){
			fprintf( stderr, "Failed to seek in file specified by fd (%d): %s", fd, strerror(errno));
			return -1;
		}
		
		// seek to the end of the file
		lseek( new_fd, length, SEEK_SET );
		
		// write out the data we setup earlier to 0 out the old part of the file
		errno = 0;
		int written = write( new_fd, to_write, write_size );
		if( written < write_size ){
			fprintf( stderr, "Warning wrote %d bytes, expected to write %d!\n", written, write_size );
		} else if ( written < 0 ){
			fprintf( stderr, "Failed to write 0 after truncated data: %s", strerror(errno) );
			return -1;
		}
		
		// clean up
		errno = 0;
		if( lseek( new_fd, store_pos, SEEK_SET ) < 0 ){
			fprintf( stderr, "Failed to seek back to original offset: %s", strerror(errno) );
		}
		
		close(new_fd);
		// call the truncate syscall to finish up
		return syscall( __NR_ftruncate, fd, length );
	}	
}
#endif