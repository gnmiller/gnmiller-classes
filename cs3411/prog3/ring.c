/*
 * NAME: Greg Miller
 * ASSIGNMENT: Program 3
 * COURSE: CS3411
 * TERM: Fall 2014
 * 
 * FILE: ring.c
 * usage: ring <num_procs> <string>
 * 		Where num_procs is in the range [2,100]
 * 		And string is an optional argument that will
 * 			prepend the children's output.
 * 
 * The file, when executed, will call fork() num_procs-1 times.
 * 		After each call to fork() the new process will attach
 * 		its stdin and stdout to the correct pipes and then call
 * 		execve( ... ) with various arguments (notably its count
 * 		in the list, and a formatted version of <string>
 * 
 * After creating the correct number of children the process
 * 		calls getch (checking for the newline character to be safe
 * 		and when '\n' is detected it sends SIGUSR1 to the first
 * 		child. Signalling it to begin work. It then waits again 
 * 		(same conditions on waiting using getch), upon receiving
 * 		a newline the parent loops num_procs-1 times sending SIGTERM
 * 		to each child.
 * 
 * NOTES:
 * -- Most error checking is lazy, it was elegant until unusual
 * 		bugs kept popping up.
 * 
 * -- dup2(new, old) is used as opposed to close(old) then dup(new) as the
 * 		functionality is the same.
 * 
 * -- Technically <string> can't be any length (max of 200).
 * 
 * -- Prolicide is the act of killing one's children.
 */

#define _XOPEN_SOURCE
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MIN_PROCS 2
#define MAX_PROCS 100

int main(int argc, char** argv){
	int prev[2], next[2], special[2], i, num_procs;
	char outStr[100], buf[100];
	
	/* never trust the user */
	if( argc < 2 ){
		errno = EINVAL;
		fprintf(stderr,"Usage: ring <num_procs> <string>\n");
		fprintf(stderr,"    Where num_procs is an integer in the range [2,100].\n");
		fprintf(stderr,"    String is a (optional) string of any length.\n");
		exit(1);
	}
	
	num_procs = atoi(argv[1]);
	int children[num_procs];
	
	/* still never trusting the user */
	if( argc > 3 ){
		errno = E2BIG;
		fprintf(stderr,"Usage: ring <num_procs> <string>\n");
		fprintf(stderr,"    Where num_procs is an integer in the range [2,100].\n");
		fprintf(stderr,"    String is a (optional) string of any length.\n");
		exit(1);
	}
	if( num_procs < MIN_PROCS || num_procs > MAX_PROCS ){
		errno = EINVAL;
		fprintf(stderr,"Usage: ring <num_procs> <string>\n");
		fprintf(stderr,"    Where num_procs is an integer in the range [2,100].\n");
		fprintf(stderr,"    String is a (optional) string of any length.\n");
		exit(1);
	}
	
	pipe(next);
	pipe(special); // used for linking first to last
	
	/* create first child */
	if( ( children[0] = fork() ) == 0 ){
		
		/* attach stdin to the pipe */
		if( dup2(special[0],STDIN_FILENO) < 0 ) exit(1);
		if( close(special[0]) ) exit(1);
		if( close(special[1]) ) exit(1);
		
		/* attach stdout to the pipe */
		if( dup2(next[1], STDOUT_FILENO) < 0 ) exit(1);
		if( close(next[0]) ) exit(1);
		if( close(next[1]) ) exit(1);
		
		/* run the new process */
		if( argc == 3 ){
			strcpy(outStr,argv[2]);
			strcat(outStr, " 0: ");
		} else {
			strcpy(outStr, "0: ");
		}
		char* new_args[4] = { "./link", outStr, "first", NULL }; // let him know hes the first guy
		char* new_env[1] = { NULL };
		execve(new_args[0], new_args, new_env);
		/* only accessible if exec fails (guaranteed error) */
		perror("exec");
		exit(1);
	}
	
	/* setup the chain */
	prev[0] = next[0];
	prev[1] = next[1];
	
	/* create n-2 children */
	for(i = 1; i < num_procs-1; ++i){
		
		pipe(next);
		/* fork child */
		if ( ( children[i] = fork() ) == 0 ){
			
			/* attach stdin to the pipe */
			if( dup2(prev[0], STDIN_FILENO) < 0 ) exit(1);
			if( close(prev[0]) ) exit(1);
			if( close(prev[1]) ) exit(1);

			/* attach stdout to the pipe */
			if( dup2(next[1], STDOUT_FILENO) < 0 ) exit(1);
			if( close(next[0]) ) exit(1);
			if( close(next[1]) ) exit(1);
			
			/* run the new process */
			if( argc == 3 ){
				strcpy(outStr,argv[2]);
				sprintf(buf, " %d: ", i);
				strcat(outStr, buf);
			} else {
				sprintf(buf, "%d: ", i);
				strcat(outStr, buf);
			}
			char* new_args[3] = { "./link", outStr, NULL }; // null terminated array of args
			char* new_env[1] = { NULL }; // null terminated array of env vars
			execve(new_args[0], new_args, new_env);
			
			/* only accessible if exec fails (guaranteed error) */
			perror("exec");
			exit(1);
		}
		
		/* close old descriptors */
		if( close(prev[0]) ) exit(1);
		if( close(prev[1]) ) exit(1);
		/* continue the chain */
		prev[0] = next[0];
		prev[1] = next[1];	
		
	}
	
	/* last child */
	if( ( children[num_procs-1] = fork() ) == 0 ){
		
		/* attach stdin to the pipe */
		if( dup2(prev[0],STDIN_FILENO) < 0 ) exit(1);
		if( close(prev[0]) ) exit(1);
		if( close(prev[1]) ) exit(1);
		
		/* attach stdout to the pipe */
		if( dup2(special[1],STDOUT_FILENO) < 0 ) exit(1);
		if( close(special[0]) ) exit(1);
		if( close(special[1]) ) exit(1);
		
		/* run the new process */
		/* create some stuff for execing */
		if( argc == 3 ){
			strcpy(outStr,argv[2]);
			sprintf(buf, " %d: ", num_procs-1);
			strcat(outStr, buf);
		} else {
			sprintf(buf, "%d: ", num_procs-1);
			strcat(outStr, buf);
		}
		char* new_args[4] = { "./link", outStr, NULL }; // null terminated array of args
		char* new_env[1] = { NULL }; // null terminated array of env vars
		execve(new_args[0], new_args, new_env);
		
		/* only accessible if exec fails (guaranteed error) */
		perror("exec");
		exit(1);
	}

	/* ensure no dangling FDs in parent that may cause hangs */
	if( close(special[0]) ) exit(1);
	if( close(special[1]) ) exit(1);
	if( close(prev[0]) ) exit(1);
	if( close(prev[1]) ) exit(1);

	/* wait for return */
	char in;
	fprintf(stdout, "Press return/enter to continue... \n");
	in = (char)getchar();
	while(in != '\n'){ // don't try to get tricky on me...
		in = (char)getchar();
		fprintf(stdout, "Press return/enter to continue... \n");
	}
	
	/* signal the child now */
	if ( kill( children[0], SIGUSR1 ) < 0 ){
 		fprintf(stderr, "failed to send the signal (SIGUSR1), please check all children have terminated manually. %s\n", strerror(errno));
 		exit(1);
 	}
 	
	/* wait for return again */
	in = 0;
	fprintf(stdout, "Press return/enter to commit prolicide... \n");
	in = (char)getchar();
	while(in != '\n'){ // don't try to get tricky on me...
		in = (char)getchar();
		fprintf(stdout, "Press return/enter to commit prolicide... \n");
	}
	
	/* kill all children off */
	int flag;
	flag = 0;
	for(i = 0; i < num_procs; ++i ){
	
		if ( kill( children[i], SIGTERM ) < 0 ){
			fprintf(stderr, "failed to send SIGTERM to child %d, continuing to attempt to kill children\n", children[0]);
			fprintf(stderr, "please enure all children have terminated manually. %s\n", strerror(errno));
			++flag;
		} else continue;
		
	}
	
	if(flag > 0) exit(flag);
	else exit(0);
	
}