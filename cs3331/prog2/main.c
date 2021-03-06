/* 
 * --------------------------------------------------------
 * Name: Greg Miller				userID: gnmiller
 * DUE DATE: 10/17/2014
 * PROGRAM 3
 * FILE NAME: main.c
 * PURPOSE:
 * 	Main program. Reads in 3 arrays of size m, n, and k
 * 		into shared memory arrays representing x[],y[],a[].
 * 		Then forks twice and execs into merge and qsort.
 * 		After forking main waits for children then prints
 * 		sorted arrays and exits.
 * --------------------------------------------------------
 */

#ifndef GNMILLER_MAIN_C
#define gnmiller_main_c

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/wait.h>

#define PARAMS_MIN 5

/* define some macros for using as keys */
#ifndef GNM_KEY
#define GNM_KEY
#define GNM_KEY_MX 1337
#define GNM_KEY_MY 1338
#define GNM_KEY_MO 1339
#define GNM_KEY_Q 1340
#endif

int main(int argc, char** argv){
	int i;
	size_t len;
	char *buf, *line, *tmp;
	int children[2];
	errno = 0;
	
	/* -- Shared Mem Vars -- */
	key_t key_m_x,key_m_y,key_m_o,key_q; // our key
	int shmid_merge_x,shmid_merge_y,shmid_merge_o,shmid_qsort;
	size_t shmsize_merge_x,shmsize_merge_y,shmsize_qsort;
	int *shm_merge_x, *shm_merge_y, *shm_merge_o, *shm_qsort;
	
	if(argc>1){
		errno = EINVAL;
		perror("invalid number of args");
		return -1;
	}
	
	if (access("./main", R_OK) != 0){
			perror("failed to touch file for keys");
			return -1;
	} // make sure that we have a file to key off
	
	/* 
	*  get some keys
	 * if datafile doesnt exist we cannot possible get to this line
	 * thus we can safely key off it and some arbitrarily defined ints
	 */
	key_m_x = ftok( "./main", GNM_KEY_MX );
	key_m_y = ftok( "./main", GNM_KEY_MY );
	key_m_o = ftok( "./main", GNM_KEY_MO );
	key_q = ftok( "./main", GNM_KEY_Q );
	
	buf = (char*)malloc(sizeof(char)*100);
	len = 0;
	
	/* read k */
	getline( &line, &len, stdin );
	shmsize_qsort = atoi(line);
	
	/* request shared memory for qsort */
	shmid_qsort = shmget(key_q, sizeof(int)*shmsize_qsort, IPC_CREAT | IPC_EXCL | 0666 );
	shm_qsort = (int*) shmat( shmid_qsort, NULL, 0 );
	
	/* print qsort shm info */
	sprintf(buf, "*** MAIN: shared memory key (qsort) = %d\n", key_q);
	write(STDOUT_FILENO, buf, strlen(buf));
	sprintf(buf, "*** MAIN: shared memory created (qsort)\n");
	write(STDOUT_FILENO, buf, strlen(buf));
	sprintf(buf, "*** MAIN: shared memory attached and is ready to use (qsort)\n");
	write(STDOUT_FILENO, buf, strlen(buf));
	// printing done
	
	/* read a[] */
	getline( &line, &len, stdin );	
	shm_qsort[0] = atoi ( strtok ( line, " \n") );
	for(i = 1; i < shmsize_qsort; ++i){
		shm_qsort[i] = atoi ( strtok ( NULL, " \n" ) );
	}
	
	/* print data related to qsort array */
	sprintf(buf, "Input array for qsort has %d elements:\n", (int)shmsize_qsort);
	write(STDOUT_FILENO, buf, strlen(buf));
	
	sprintf(buf, "   ");
	tmp = (char*)malloc(sizeof(char)*100);
	for(i = 0; i < shmsize_qsort; ++i){
		sprintf(tmp, "%d ", shm_qsort[i]);
		strcat(buf,tmp);
	} strcat(buf, "\n");
	free(tmp);
	write(STDOUT_FILENO, buf, strlen(buf));
	// print done
	
	/* ------------------------------------------------------------------*/
	
	/* get m */
	getline( &line, &len, stdin );
	shmsize_merge_x = atoi(line);
	
	/* request shared memory for merge_array_x */
	shmid_merge_x = shmget(key_m_x, sizeof(int)*shmsize_merge_x, IPC_CREAT | IPC_EXCL | 0666 );
	shm_merge_x = (int*)shmat( shmid_merge_x, NULL, 0 );
	
	/* print shm info for merge array x[] */
	sprintf(buf, "*** MAIN: shared memory key (merge_array_x) = %d\n", key_m_x);
	write(STDOUT_FILENO, buf, strlen(buf));
	sprintf(buf, "*** MAIN: shared memory created (merge_array_x)\n");
	write(STDOUT_FILENO, buf, strlen(buf));
	sprintf(buf, "*** MAIN: shared memory attached and is ready to use (merge_array_x)\n");
	write(STDOUT_FILENO, buf, strlen(buf));
	
	/* read x[] */
	getline( &line, &len, stdin );	
	shm_merge_x[0] = atoi ( strtok ( line, " \n" ) );
	for(i = 1; i < shmsize_merge_x; ++i){
		shm_merge_x[i] = atoi ( strtok ( NULL, " \n" ) );
	}
	
	/* print data related to merge array  x */
	sprintf(buf, "Input array for mergesort (x) has %d elements:\n", (int)shmsize_merge_x);
	write(STDOUT_FILENO, buf, strlen(buf));
	
	sprintf(buf, "   ");
	tmp = (char*)malloc(sizeof(char)*100);
	for(i = 0; i < shmsize_merge_x; ++i){
		sprintf(tmp, "%d ", shm_merge_x[i]);
		strcat(buf,tmp);
	} strcat(buf, "\n");
	free(tmp);
	write(STDOUT_FILENO, buf, strlen(buf));
	// print done
	
	/* get n */
	getline( &line, &len, stdin );
	shmsize_merge_y = atoi(line);
	
	/* request shared memory for merge_array_y */
	shmid_merge_y = shmget(key_m_y, sizeof(int)*shmsize_merge_y, IPC_CREAT | IPC_EXCL | 0666 );
	shm_merge_y = (int*)shmat( shmid_merge_y, NULL, 0 );
	
	/* print shm info for merge array y[] */
	sprintf(buf, "*** MAIN: shared memory key (merge_array_y) = %d\n", key_m_y);
	write(STDOUT_FILENO, buf, strlen(buf));
	sprintf(buf, "*** MAIN: shared memory created (merge_array_y)\n");
	write(STDOUT_FILENO, buf, strlen(buf));
	sprintf(buf, "*** MAIN: shared memory attached and is ready to use (merge_array_y)\n");
	write(STDOUT_FILENO, buf, strlen(buf));
	//print done
	
	/* read y[] */
	getline( &line, &len, stdin );	
	shm_merge_y[0] = atoi ( strtok ( line, " \n") );
	for(i = 1; i < shmsize_merge_y; ++i){
		shm_merge_y[i] = atoi ( strtok ( NULL, " \n" ) );
	}
		
	/* print data related to merge array  x */
	sprintf(buf, "Input array for mergesort (y) has %d elements:\n", (int)shmsize_merge_y);
	write(STDOUT_FILENO, buf, strlen(buf));
	
	sprintf(buf, "   ");
	tmp = (char*)malloc(sizeof(char)*100);
	for(i = 0; i < shmsize_merge_y; ++i){
		sprintf(tmp, "%d ", shm_merge_y[i]);
		strcat(buf,tmp);
	} strcat(buf, "\n");
	free(tmp);
	write(STDOUT_FILENO, buf, strlen(buf));
	// print done
	
	/* ------------------------------------------------------------------*/
	
	/* request shared memory for merge array output */
	shmid_merge_o = shmget(key_m_o, ( sizeof(int)*shmsize_merge_x + sizeof(int)*shmsize_merge_y ), IPC_CREAT | IPC_EXCL | 0666 );
	//
	
	/* print shm info for merge array output */
	sprintf(buf, "*** MAIN: shared memory key (merge_array_output) = %d\n", key_m_o);
	write(STDOUT_FILENO, buf, strlen(buf));
	sprintf(buf, "*** MAIN: shared memory created (merge_array_output)\n");
	write(STDOUT_FILENO, buf, strlen(buf));
	sprintf(buf, "*** MAIN: shared memory attached and is ready to use (merge_array_output)\n");
	write(STDOUT_FILENO, buf, strlen(buf));
	// print done

	free(line);
	
	shmdt(shm_merge_x);				
	shmdt(shm_merge_y);
	shmdt(shm_qsort);
	
	if ( ( children[0] = fork() ) == 0 ){ // merge
	
		sprintf(buf, "*** MAIN spawning process for mergesort\n");
		write(STDOUT_FILENO, buf, strlen(buf));
		
		char m[10];
		char n[10];
		sprintf(m, "%d", (int)shmsize_merge_x);
		sprintf(n, "%d", (int)shmsize_merge_y);
		
		char* n_argv[4] = { "./merge", m, n, NULL };		
		execvp(n_argv[0], n_argv);

	} else if ( ( children[1] = fork() ) == 0 ){ // qsort
		
		sprintf(buf, "*** MAIN spawning process for quicksort\n");
		write(STDOUT_FILENO, buf, strlen(buf));
		
		char t[10];		
		char left[10];
		char right[10];
		sprintf(t, "%d", (int)shmsize_qsort);
		sprintf(left, "%d", 0);
		sprintf(right, "%d", (int)shmsize_qsort-1);
		
		char* n_argv[5] = { "./qsort", left, right, t, NULL };		
		execvp(n_argv[0], n_argv);	
		
	} else { // parent
		int status;
		int childPID;
		/* wait for our children and save whoever exited's PID */
		//sleep(1);
		while( (childPID = waitpid(-1, &status, 0 ) ) > 0 ){
			
			if( childPID == children[1] ){ // qsort exited
				shm_qsort = shmat( shmid_qsort, NULL, 0 );
				
				/* print the outputed array from qsort */
				sprintf(buf, "*** MAIN: sorted output from quicksort:\n", status);
				write(STDOUT_FILENO, buf, strlen(buf));
				
				sprintf(buf, "   ");
				tmp = (char*)malloc(sizeof(char)*100);
				for(i = 0; i < shmsize_qsort; ++i){
					sprintf(tmp, "%d ", shm_qsort[i]);
					strcat(buf,tmp);
				} strcat(buf, "\n");
				free(tmp);
				write(STDOUT_FILENO, buf, strlen(buf));
				
			} else  if( childPID == children[0] ){ // merge exited
				
				/* print the outputted array from merge sort */
				shm_merge_o = shmat( shmid_merge_o, NULL, 0 );
				
				sprintf(buf, "*** MAIN: sorted output from mergesort:\n");
				write(STDOUT_FILENO, buf, strlen(buf));
				
				sprintf(buf, "   ");
				tmp = (char*)malloc(sizeof(char)*100);
				for(i = 0; i < shmsize_merge_x + shmsize_merge_y; ++i){
					sprintf(tmp, "%d ", shm_merge_o[i]);
					strcat(buf,tmp);
				} strcat(buf, "\n");
				free(tmp);
				write(STDOUT_FILENO, buf, strlen(buf));			
			} 
		}
		
		shmdt(shm_merge_x);
		shmctl(shmid_merge_x, IPC_RMID, NULL);
				
		shmdt(shm_merge_y);
		shmctl(shmid_merge_y, IPC_RMID, NULL);
				
		shmdt(shm_merge_o);
		shmctl(shmid_merge_o, IPC_RMID, NULL);
		
		sprintf(buf, "*** MAIN: shared memory (merge_array_x) successfully detached\n");
		write(STDOUT_FILENO, buf, strlen(buf));
		sprintf(buf, "*** MAIN: shared memory (merge_array_x) successfully removed\n");
		write(STDOUT_FILENO, buf, strlen(buf));
		
		sprintf(buf, "*** MAIN: shared memory (merge_array_y) successfully detached\n");
		write(STDOUT_FILENO, buf, strlen(buf));
		sprintf(buf, "*** MAIN: shared memory (merge_array_y) successfully removed\n");
		write(STDOUT_FILENO, buf, strlen(buf));		
				
		sprintf(buf, "*** MAIN: shared memory (merge_array_output) successfully detached\n");
		write(STDOUT_FILENO, buf, strlen(buf));
		sprintf(buf, "*** MAIN: shared memory (merge_array_output) successfully removed\n");
		write(STDOUT_FILENO, buf, strlen(buf));
		
		shmdt(shm_qsort);
		shmctl(shmid_qsort, IPC_RMID, NULL);
		
		sprintf(buf, "*** MAIN: shared memory (qsort) successfully detached\n");
		write(STDOUT_FILENO, buf, strlen(buf));
		sprintf(buf, "*** MAIN: shared memory (qsort) successfully removed\n");
		write(STDOUT_FILENO, buf, strlen(buf));
	}
}

#endif