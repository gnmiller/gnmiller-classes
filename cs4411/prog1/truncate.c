/*
 * Greg Miller	gnmiller
 *
 * Program 1
 * CS4411
 * Due: 1/19/2015
 * truncate.c
 * 	Wraps the system call truncate( path, length )
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
int truncate( const char *path, off_t length ){
	struct stat f_info;
	
	#ifdef GNM_DEBUG
	printf("MODIFIED TRUNCATE\n");
	#endif
	
	/* stat the file to get size of other info */
	errno = 0;
	if ( stat( path, &f_info ) != 0 ) {
		fprintf( stderr, "Failed to stat file (%s): %s\n", path, strerror(errno));
		return -1;
	}
	
	int cur_max = f_info.st_size;
	int write_size = cur_max - length;
	
	errno = 0;
	int path_fd = open( path, O_WRONLY );
	if ( path_fd < 0 ){
		fprintf( stderr, "Failed to open file (%s) for writing: %s\n", path, strerror(errno));
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
		return syscall( __NR_truncate, path, length );		
	} else {
		int to_write[length];
		int store_pos;
		// create the data to write to 0 out the file
		memset( to_write, 0, length );
		
		// store the current file offset
		store_pos = lseek( path_fd, 0 , SEEK_CUR );
		if( store_pos < 0 ){
			fprintf( stderr, "Failed to seek in file (%s): %s", path, strerror(errno));
			return -1;
		}
		
		// seek to the end of the file
		lseek( path_fd, length, SEEK_SET );
		
		// write out the data we setup earlier to 0 out the old part of the file
		int written = write( path_fd, to_write, write_size );
		if( written < write_size ){
			fprintf( stderr, "Warning wrote %d bytes, expected to write %d!\n", written, write_size );
		}
		
		// clean up
		if( lseek( path_fd, store_pos, SEEK_SET ) < 0 ){
			fprintf( stderr, "Failed to seek back to original offset: %s", strerror(errno) );
		}
		
		// call the truncate syscall to finish up
		return syscall( __NR_truncate, path, length );
	}	
}
#endif