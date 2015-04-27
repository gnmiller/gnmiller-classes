/* 
 * --------------------------------------------------------
 * Name: Greg Miller				userID: gnmiller
 * DUE DATE: 10/17/2014
 * PROGRAM 3
 * FILE NAME: merge.c
 * PURPOSE:
 * 		merge has two primary functions. First is checks if
 * 		it was forked from main (denoted by arguements). If
 * 		it was it then forks m+n times and execs each child
 * 		into merge with an specific index. Each child then
 * 		searches the unsorted array finding its position,
 * 		once found it writes that value into the correct
 * 		index in the output shared memory array. *
 * --------------------------------------------------------
 */


#ifndef GNMILLER_QSORT_C
#define GNMILLER_QSORT_C

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/wait.h>

#ifndef GNM_KEY
#define GNM_KEY
#define GNM_KEY_MX 1337
#define GNM_KEY_MY 1338
#define GNM_KEY_MO 1339
#endif

int main(int argc, char** argv){
	int i;
	char* buf;
	
	if(argc == 3){ // i am the adjudicator
	
		int size_x, size_y;
		size_x = atoi(argv[1]);
		size_y = atoi(argv[2]);
		
		buf = (char*)malloc(sizeof(char)*100);
		
		for(i = 0; i < size_x; ++i){
		
			if ( fork() == 0 ){ // create m children
				
				sprintf(buf, "%d", i);
				char* n_argv[7] = { argv[0], argv[1], argv[2], buf, "x", "child", NULL };
				//free(buf);
				execvp(argv[0], n_argv);
				
			} else continue;
			
		}
		
		for(i = 0; i < size_y; ++i){
		
			if( fork() == 0 ){ // create n children
				
				sprintf(buf, "%d", i);
				char* n_argv[7] = { argv[0], argv[1], argv[2], buf, "y", "child", NULL };
				//free(buf);
				execvp(argv[0], n_argv);
				
			} else continue;
			
		}
		
		// wait for all children, if we dont wait main will think were done before merge is...
		int status;
		while ( waitpid(-1, &status, 0 ) > 0 );
		exit(0);
		
	} else { // i am going to do some sorting
		
		buf = (char*)malloc(sizeof(char)*100);
		
		key_t key_m_x, key_m_y, key_m_o;
		key_m_x = ftok( "./main", GNM_KEY_MX );
		key_m_y = ftok( "./main", GNM_KEY_MY );
		key_m_o = ftok( "./main", GNM_KEY_MO );
		
		int myval,myindex,size_x, size_y, size_o,shmid_merge_x,shmid_merge_y,shmid_merge_o;
		int *shm_merge_x, *shm_merge_y, *shm_merge_o;
		size_x = atoi(argv[1]);
		size_y = atoi(argv[2]);
		size_o = size_x+size_y;
		myindex = atoi(argv[3]);
		
		/* request shared memory for merge_array_x */
		shmid_merge_x = shmget(key_m_x, sizeof(int)*size_x, 0666 );
		shm_merge_x = (int*)shmat( shmid_merge_x, NULL, 0 );
		
		/* request shared memory for merge_array_x */
		shmid_merge_y = shmget(key_m_y, sizeof(int)*size_y, 0666 );
		shm_merge_y = (int*)shmat( shmid_merge_y, NULL, 0 );
		
		/* request shared memory for merge array output */
		shmid_merge_o = shmget(key_m_o, ( sizeof(int)*size_x + sizeof(int)*size_y ), 0666 );
		shm_merge_o = (int*)shmat( shmid_merge_o, NULL, 0 );
		
		if( strcmp(argv[4],"x") ){ // we are from array x and iterate array y
			
			myval = shm_merge_x[myindex];
			sprintf(buf, "      $$$ MERGE (%d): handling x[%d] = %d\n", getpid(), myindex, myval);
			write(STDOUT_FILENO, buf, strlen(buf));
			
			i=0;
			while( shm_merge_y[i] < myval && i < size_y ) ++i; // search for out index

			/* determine what message to output */
			if(i == 0){
				sprintf(buf, "      $$$ MERGE (%d): x[%d] = %d is found to be smaller than y[0] = %d.\n", getpid(), myindex, myval, shm_merge_y[0]);
				write(STDOUT_FILENO, buf, strlen(buf));
			} else if (i == size_y){
				sprintf(buf, "      $$$ MERGE (%d): x[%d] = %d is found to be larger than y[%d] = %d.\n", getpid(), myindex, myval, i, shm_merge_y[0]);
				write(STDOUT_FILENO, buf, strlen(buf));
			} else {
				sprintf(buf, "      $$$ MERGE (%d): x[%d] = %d is found to be between y[%d] = %d and y=[%d] and %d\n", getpid(), myindex, myval, i, shm_merge_y[i], i+1, shm_merge_y[i+1]);
				write(STDOUT_FILENO, buf, strlen(buf));
			}
			
			sprintf(buf, "      $$$ MERGE (%d): writing x[%d] = %d into index %d of the output array\n", getpid(), myindex, myval, i+myindex);
			write(STDOUT_FILENO, buf, strlen(buf));
			
			shm_merge_o[i+myindex] = myval; //write myself in
			
			shmdt(shm_merge_x);
			shmdt(shm_merge_y);
			shmdt(shm_merge_o);
			
			exit(0);
		
		} else if ( strcmp(argv[4],"y") ){ // we are from y and need to search x
			
			myval = shm_merge_y[myindex];
			sprintf(buf, "      $$$ MERGE (%d): handling y[%d] = %d\n", getpid(), myindex, myval);
			write(STDOUT_FILENO, buf, strlen(buf));
			
			i=0;
			while( shm_merge_x[i] < myval && i < size_x ) ++i; // search for out index
			
			/* determine what message to output */
			if(i == 0){
				sprintf(buf, "      $$$ MERGE (%d): y[%d] = %d is found to be smaller than x[0] = %d.\n", getpid(), myindex, myval, shm_merge_x[0]);
				write(STDOUT_FILENO, buf, strlen(buf));
			} else if (i == size_x){
				sprintf(buf, "      $$$ MERGE (%d): y[%d] = %d is found to be larger than x[%d] = %d.\n", getpid(), myindex, myval, i, shm_merge_x[0]);
				write(STDOUT_FILENO, buf, strlen(buf));
			} else {
				sprintf(buf, "      $$$ MERGE (%d): y[%d] = %d is found to be between x[%d] = %d and x = [%d] and %d\n", getpid(), myindex, myval, i, shm_merge_x[i], i+1, shm_merge_x[i+1]);
				write(STDOUT_FILENO, buf, strlen(buf));
			}
			
			sprintf(buf, "      $$$ MERGE (%d): writing y[%d] = %d into index %d of the output array\n", getpid(), myindex, myval, i+myindex);
			write(STDOUT_FILENO, buf, strlen(buf));
			
			shm_merge_o[i+myindex] = myval; //write myself in
			
			shmdt(shm_merge_x);
			shmdt(shm_merge_y);
			shmdt(shm_merge_o);
			
			exit(0);
		}
	}
}

#endif