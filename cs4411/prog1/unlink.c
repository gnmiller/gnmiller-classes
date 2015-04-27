/*
 * Greg Miller	gnmiller
 *
 * Program 1
 * CS4411
 * Due: 1/19/2015
 * unlink.c
 * 	Wraps the system call unlink( path )
 *		Writes a pattern (all 0 in this case) over the data
 *			specified by the path and then calls  unlink(...).
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
int unlink( const char *pathname ){

	#ifdef GNM_DEBUG
	printf("MODIFIED UNLINK\n");
	#endif
	
	/* get some info on the file */
	struct stat f_info;
	if ( stat( pathname, &f_info ) != 0 ) {
		fprintf( stdout, "Failed to stat file (%s): %s\n", pathname, strerror(errno));
		return -1;
	}
	
	/* setup some stuff for writing 0 */
	int f_size = f_info.st_size;
	int to_Write[f_size];
	memset( to_Write, 0, f_size );
	
	/* open the file for writing */
	int fd = open( pathname, O_WRONLY );
	if( fd < 0 ){
		fprintf( stderr, "Failed to open file (%s) for writing: %s", pathname, strerror(errno) );
		return -1;
	}
	
	/* write to the file */
	int written;
	written = write( fd, to_Write, f_size );
	if( written < f_size ) fprintf( stderr, "Warning, wrote %d bytes but expected to write %d!\n", written, f_size );
	
	/* call unlink */
	return syscall( __NR_unlink, pathname );
	
}
#endif