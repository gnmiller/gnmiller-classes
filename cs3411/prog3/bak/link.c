#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>

volatile int flag;

static void catch_sigusr1(int signum){
	flag = 1;
}

int main(int argc, char** argv){
	flag = 0;
	
	/* assign the sig handler */
	if( signal(SIGUSR1, catch_sigusr1) == SIG_ERR ){
		fprintf(stderr, "failed to assign the signal handler. %s\n", strerror(errno));
		exit(1);
	}
	
	/* we are the first guy because we have more args */
	if(argc == 3){
		/* wait for the signal */
		while(!flag);
		
		/* write out 0 */
		if( write(STDOUT_FILENO, 0, sizeof(int)) < 0 ){
			fprintf(stderr, "failed to read. %s\n", strerror(errno));
			exit(1);
		}
		
	}
	
	char ch[10], buf[200], tmp[200];
	int temp, me;
	me = atoi( argv[2] );
	/* inf loop of reads and writes until we get killed */
	while(1){
		
		if ( read(STDIN_FILENO, &ch, sizeof(int)) < 0 ){
			fprintf(stderr, "failed to read. %s\n", strerror(errno));
			exit(1);
		}
		
		temp = atoi(ch);
		++temp;
		sleep(1);
		
		/* build a string to output to stderr */
		strcpy(buf, argv[1]);
		sprintf(tmp, ": pid = %d, value = %d\n", (int)getpid(), temp);
		strcat(buf, tmp);
		if( write(STDERR_FILENO, buf, strlen(buf)) < 0 ){
			fprintf(stderr, "failed to write. %s\n", strerror(errno));
			exit(1);
		}
		
		if( write(STDOUT_FILENO, &temp, sizeof(int)) < 0 ){
			fprintf(stderr, "failed to write. %s\n", strerror(errno));
			exit(1);
		}
		
	}
	
}