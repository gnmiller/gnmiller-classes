/* 
 * --------------------------------------------------------
 * Name: Greg Miller				userID: gnmiller
 * DUE DATE: 10/17/2014
 * PROGRAM 3
 * FILE NAME: qsort.c
 * PURPOSE:
 * 	qsort has two functions. First it checks if it is the
 * 		master qsort (called by main). If it is it partitions
 * 		the shared memory array into a[0 ... m]
 * 		and a[m+1 ... r]. Then forks and execs two more qsorts
 * 		The subsequent children check if they are children
 * 		then recursive call quicksort( ... ) sorting each
 * 		half of the array they are passed.
 * 		After children complete the parent merges the two
 * 		halves and exits.
 * --------------------------------------------------------
 */


#ifndef GNMILLER_QSORT_C
#define GNMILLER_QSORT_C

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <error.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/wait.h>

#ifndef GNM_KEY
#define GNM_KEY
#define GNM_KEY_Q 1340
#endif

void quickSort(int*,int,int);
int partition(int*,int,int);

int main(int argc, char** argv){
	
	int shmid_qsort;
	int* shm_qsort;
	
	if(argc <= 4){ // initial run
		// get the partition value
		int m,i;
		int children[2], left, right;
		char* n_argv[6];
		char* buf;
		
		left = atoi(argv[1]);
		right = atoi(argv[2]);
		m = right/2; // partition
		shm_qsort = NULL;
		
		buf = (char*)malloc(sizeof(char)*100);
		sprintf(buf, "   ### QSORT (%d): started with a[%d ... %d]\n", getpid(), left, right);
		write(STDOUT_FILENO, buf, strlen(buf));
		sprintf(buf, "   ### QSORT (%d): initial partition: %d\n", getpid(), m);
		write(STDOUT_FILENO, buf, strlen(buf));
		
		for(i = 0; i < 3; ++i){
			n_argv[i] = (char*)malloc(sizeof(char)*32); // arbitrarily sized...
		}
		
		strcpy(n_argv[0],"./qsort");		
		
		// create 2 children and wait for them
		if( ( children[0] = fork() ) == 0 ){ // left child
			
			sprintf(n_argv[1], "%d", left);
			sprintf(n_argv[2], "%d", m);
			n_argv[3] = argv[3];
			n_argv[4] = "child";
			n_argv[5] = NULL;
			execvp(n_argv[0],n_argv); // exec into qsort (left)
			
		} else if ( ( children[1] = fork() ) == 0 ){ // right child
			
			sprintf(n_argv[1], "%d", m+1);
			sprintf(n_argv[2], "%d", right);
			n_argv[3] = argv[3];
			n_argv[4] = "child";
			n_argv[5] = NULL;
			execvp(n_argv[0],n_argv); // exec into qsort (right)
			
		} else { // parent
			
			int status;
			while ( waitpid(-1, &status, 0 ) > 0 );
			
			
			/* attach shm to this process */
			key_t key_q;
			key_q = ftok( "./main", GNM_KEY_Q );
			shmid_qsort = shmget(key_q, sizeof(int)*right, 0666 );
			shm_qsort = (int*)shmat( shmid_qsort, NULL, 0 );
			
			sprintf( buf, "   ### QSORT (%d): Children done sorting. Merging arrays.\n", getpid() );
			write(STDOUT_FILENO, buf, strlen(buf));
			
			quickSort(shm_qsort, 0, right); // merge the arrays
			
			shmdt(shm_qsort);
			
			exit(0);
			
		}
	} else { // children come here
		
		int left,right,size;
		left = atoi(argv[1]);
		right = atoi(argv[2]);
		size = atoi(argv[3]);
		char* buf;
		
		/* attach shm to this process */
		key_t key_q;
		key_q = ftok( "./main", GNM_KEY_Q );
		shmid_qsort = shmget(key_q, sizeof(int)*size, 0666 );
		shm_qsort = (int*)shmat( shmid_qsort, NULL, 0 );
		
		buf = (char*)malloc(sizeof(char)*100);
		sprintf(buf, "   ### QSORT CHILD(%d): started with a[%d ... %d]\n", getpid(), left, right);
		write(STDOUT_FILENO, buf, strlen(buf));
		
		quickSort( shm_qsort, left, right );
		
		sprintf(buf, "   ### QSORT (%d): exiting.\n", getpid());
		write(STDOUT_FILENO, buf, strlen(buf));
		
		shmdt(shm_qsort);
		exit(0);
	}
}


/* --------------------------------------------------------
 * FUNCTION: quickSort
 * 		A recursive implementation of the quick sort algorithm.
 * PARAMETER USAGE:
 * 		a: The array to sort
 * 		l: The leftmost index of the array to sort
 * 		r: The rightmost index of the array to sort
 * FUNCTIONS CALLED:
 * 		partition( ... )
 * --------------------------------------------------------
 */
void quickSort( int a[], int l, int r)
{
   int j;

   if( l < r ) 
   {
   	// divide and conquer
       j = partition( a, l, r);
       quickSort( a, l, j-1);
       quickSort( a, j+1, r);
   }
	
}

/* --------------------------------------------------------
 * FUNCTION: partition
 * 		Calculate a pivot point in the array and then
 * 		reorder the array such that all values < the pivot
 * 		are left of it and all values > the pivot are right
 * 		of it.
 *
 * NOTE: The pivot is ALWAYS a[l]
 *
 * PARAMETER USAGE:
 * 		a: The array we are partitioning
 * 		l: The leftmost index of the array to sort
 * 		r: The rightmost index of the array to sort
 * FUNCTIONS CALLED:
 * 		n/a
 * --------------------------------------------------------
 */
int partition( int a[], int l, int r) {
   int pivot, i, j, t;
   pivot = a[l];
   i = l; j = r+1;
		
   while( 1)
   {
   	do ++i; while( a[i] <= pivot && i <= r );
   	do --j; while( a[j] > pivot );
   	if( i >= j ) break;
   	t = a[i]; a[i] = a[j]; a[j] = t;
   }
   t = a[l]; a[l] = a[j]; a[j] = t;
   return j;
}
#endif