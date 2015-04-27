#define _XOPEN_SOURCE
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char** argv){
	int prev[2], next[2], special[2], i, num_procs;
	char buf[200], outStr[100];
	
	num_procs = atoi(argv[1]);
	int children[num_procs];
	
	pipe(next);
	pipe(special); // used for linking first to last
	
	/* create first child */
	if( ( children[0] == fork() ) == 0 ){
		
		if(special){
			dup2(special[0],STDIN_FILENO); // put read to stdin
			close(special[0]);
			close(special[1]);
		} else {
			fprintf(stderr, "failed to assign first pipe's stdin. %s", strerror(errno));
			exit(1);
		}
		
		if(next){
			dup2(next[1],STDOUT_FILENO); // write to stdout
			close(next[0]);
			close(next[1]);
		} else {
			fprintf(stderr, "failed to assign first pipe's stdout %s", strerror(errno));
			exit(1);
		}
		
		/* run the new process */
		if (argc == 2) sprintf(outStr, "%s 0");
		else sprintf(outStr, "0");
		char* new_args[4] = { "./ring", outStr, "first", NULL }; // let him know hes the first guy
		char* new_env[1] = { NULL };
		execve(new_args[0], new_args, new_env);
		
	}
	
	/* setup the chain */
	prev[0] = next[0];
	prev[1] = next[1];
	
	/* create n-2 children */
	for(i = 0; i < num_procs - 1; ++i){
		
		pipe(next);
		
		/* fork child */
		if ( ( children[i] = fork() ) == 0 ){
			
			if(prev){
				dup2(prev[0],STDIN_FILENO); // put read to stdin
				close(prev[0]);
				close(prev[1]);
			} else {
				fprintf(stderr, "child %d (%d) failed to attach read pipe to stdin. %s\n", i, getpid(), strerror(errno));
				exit(1);
			}
		
			if(next){
				dup2(next[1],STDOUT_FILENO); // put read to stdin
				close(next[0]);
				close(next[1]);
			} else {
				fprintf(stderr, "child %d (%d) failed to attach write pipe to stdin. %s\n", i, getpid(), strerror(errno));
				exit(1);
			}
			
			/* run the new process */
			/* create some stuff for execing */
			if (argc == 2) sprintf(outStr, "%s %d", i);
			else sprintf(outStr, "%d", i);
			char* new_args[4] = { "./ring", outStr, NULL }; // null terminated array of args
			char* new_env[1] = { NULL }; // null terminated array of env vars
			execve(new_args[0], new_args, new_env);
			
		}
		
		/* close old descriptors */
		close(prev[0]);
		close(prev[1]);
		/* continue the chain */
		prev[0] = next[0];
		prev[1] = next[1];	
		
	}
	
	/* last child */
	if( ( children[num_procs-1] = fork() ) == 0 ){
		if(prev){
			dup2(prev[0],STDIN_FILENO); // put read to stdin
			close(prev[0]);
			close(prev[1]);
		} else {
			fprintf(stderr, "failed to attach last child's read. %s\n", i, getpid(), strerror(errno));
			exit(1);
		}
		
		if(special){
			dup2(special[1],STDOUT_FILENO); // put read to stdin
			close(special[0]);
			close(special[1]);
		} else {
			fprintf(stderr, "failed to attach last child's write. %s\n", i, getpid(), strerror(errno));
			exit(1);
		}
		
		/* run the new process */
		/* create some stuff for execing */
		char* new_args[4] = { "./ring", outStr, NULL }; // null terminated array of args
		char* new_env[1] = { NULL }; // null terminated array of env vars
		execve(new_args[0], new_args, new_env);
		
	}
	
	/* ensure no dangling FDs in parent that may cause hangs */
	close(special[0]);
	close(special[1]);
	close(prev[0]);
	close(prev[1]);
	close(next[0]);
	close(next[1]);
	
	/* wait for return */
	char in;
	in = (char)getchar();
	fprintf(stdout, "Press return/enter to continue... \n");
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
	in = (char)getchar();
	fprintf(stdout, "Press return/enter to kill... \n");
	while(in != '\n'){ // don't try to get tricky on me...
		in = (char)getchar();
		fprintf(stdout, "Press return/enter to kill... \n");
	}
	
	/* kill all children off */
	int flag;
	flag = 0;
	for(i = 0; i < num_procs; ++i ){
	
		if ( kill( children[i], SIGUSR1 ) < 0 ){
			fprintf(stderr, "failed to send SIGTERM to child %d, continuing to attempt to kill children\n");
			fprintf(stderr, "please enure all children have terminated manually. %s\n", strerror(errno));
			++flag;
		} else continue;
		
	}
	
	if(flag > 0) exit(flag);
	else exit(0);
	
}