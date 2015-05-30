/*
 * NAME: Greg Miller
 * ASSIGNMENT: Program 3
 * COURSE: CS3411
 * TERM: Fall 2014
 * 
 * FILE: link.c
 * usage: should not be invoked directly.
 * expected arguments from parent:
 * 		link <string> <my count> <flag>
 * 		
 * link is executed via ring with its arguments consisting
 * 		of its count in the list, a string and (if first)
 * 		a flag.
 * 
 * If the flag is specified ONLY this process will enter
 * 		into an infinite loop based on the global variable
 * 		flag. When the process receives SIGUSR1 the signal
 * 		handler is invoked, assigning flag to 1, causing
 * 		causing the process to exit the loop and write 1
 * 		to stdout. Then the process proceeds as with all
 * 		others.
 * 
 * If flag is not specified (or SIGUSR1 has been received)
 * 		the process enters into an infinite loop. On each
 * 		iteration the process attempts to read from stdin
 * 		one integer, add one to it then write it to
 * 		stdout. After which it prints a string containing
 * 		its PID, counter, a string specified in the arguments
 * 		and the integer that it read (after it has added to it).
 * 
 * NOTES:
 * -- There is no exit condition for children other than
 * 		receiving a signal that would cause termination through
 * 		its default action (ex SIGTERM). The parent will attempt to
 *		SIGTERM all children but in the event of errors it should 
 * 		be ensured that all children are killed manually.
 * 
 * -- As per the project specifications (under completed output info)
 * 		the first process in the chain (child 0) does not output any
 * 		info the stderr after writing to stdout.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>

#define SLEEP_TIME 1

/* int to use for signalling when the first child is to write to STDOUT */
volatile int flag;

/*
 * Signal handler
 * 
 * @param The signal to catch.
 * @return 
 */
static void catch_sigusr1(int signum){
	flag = 1;
	return;
}

int main(int argc, char** argv){
	flag = 0;
	
	/* 
	 * assign the sig handler for SIGUSR 1
	 * since this returns a struct we check for the error it reports
	 * instead of 0, <0, etc
	 */
	if( signal(SIGUSR1, catch_sigusr1) == SIG_ERR ){
		fprintf(stderr, "failed to assign the signal handler. %s\n", strerror(errno));
		exit(1);
	}
	
	/* we are the first guy because we have more args */
	if(argc == 3){
		/* wait for the signal */
		while(!flag);
		
		/* write out 0 */
		int zed;
		zed = 0;
		if( write(STDOUT_FILENO, &zed, sizeof(int)) < 0 ){
			fprintf(stderr, "failed to write (%d). %s\n", getpid(), strerror(errno));
			exit(1);
		}
		
	}
	
	char buf[200], tmp[200];
	int count;

	/* inf loop of reads and writes until we get killed */
	for(;;){
		
		/* read the counter */
		if ( read(STDIN_FILENO, &count, sizeof(int)) < 0 ){
			fprintf(stderr, "failed to read (%d). %s\n", getpid(), strerror(errno));
			exit(1);
		}
		
		count+=1;
		sleep(SLEEP_TIME); // not a system call...
		
		/* build a string to output to stderr */
		strcpy(buf, argv[1]);
		sprintf(tmp, "pid = %d, value = %d\n", getpid(), count);
		strcat(buf, tmp);
		
		/* write the new count out to the next guy */
		if( write(STDERR_FILENO, buf, strlen(buf)) < 0 ){
			fprintf(stderr, "failed to write (%d). %s\n", getpid(), strerror(errno));
			exit(1);
		}
		
		/* print out our info to the terminal (or where ever stderr happens to be going) */
		if( write(STDOUT_FILENO, &count, sizeof(int)) < 0 ){
			fprintf(stderr, "failed to write (%d). %s\n", getpid(), strerror(errno));
			exit(1);
		}
		
	}
	
}
