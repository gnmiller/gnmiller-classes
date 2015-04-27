// ----------------------------------------------------------- 
// NAME : Greg Miller                         User ID: gnmiller 
// DUE DATE : 9/26/14                                      
// PROGRAM ASSIGNMENT 1                                        
// FILE NAME : prog1.c
// PROGRAM PURPOSE :                                           
//		Compute and output various values and algoirthms using 
//		fork() to spawn children to divide the work needed to
//		be done.
// -----------------------------------------------------------
#ifndef gnmiller_prog1_c
#define gnmiller_prog1_c

#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

#define PI 3.14159265358979323846
#include <math.h>

#include <stdio.h>
#include <string.h>

// Define 2 macros for usage in heapSort.
// IS_LESS ... returns the lesser of the values v1, and v2
// SWAP ... sets r = s and s = r
#define IS_LESS(v1, v2)  (v1 < v2)
#define SWAP(r,s)  do{int t=r; r=s; s=t; } while(0)

// function defs
void heapSort(int* heap, int size);
void heapDown(int* heap, int start, int size);
long fibonacci(long n);

int main(int argc, char** argv){
	int r, m, n, s,buf_max,i;
	int children[4]; // we don't really NEED this since we dont care what child PIDs are
	char* buf;

	r = m = n = s = 0;

	if(argc!=5){
		
		buf = "Invalid # of characters in the arguments.\n";
		write(STDOUT_FILENO,buf,strlen(buf));
		return -1;
		
	} else if (argv[1] < 0 || argv[2] < 0 || argv[3] < 0 || argv[4] < 0 ){
		
		buf = "Invalid input (negative number).\n";
		write(STDOUT_FILENO,buf,strlen(buf));
		return -1;
		
	}
	
	buf = "Main process started\n";
	write(STDOUT_FILENO,buf,strlen(buf));

	m = atoi(argv[1]);
	n = atoi(argv[2]);
	r = atoi(argv[3]);
	s = atoi(argv[4]);
	
	buf = malloc(sizeof(char)*100);	
	
	sprintf(buf,"Number of Random Numbers   = %d\n",m);
	write(STDOUT_FILENO,buf,strlen(buf));
	
	sprintf(buf,"Fibonacci Input            = %d\n",n);
	write(STDOUT_FILENO,buf,strlen(buf));
	
	sprintf(buf,"Buffon's Needle Iterations = %d\n",r);
	write(STDOUT_FILENO,buf,strlen(buf));
	
	sprintf(buf,"Integration Iterations     = %d\n",s);
	write(STDOUT_FILENO,buf,strlen(buf));
	
	sprintf(buf,"Heap Sort Process Created\n");
	write(STDOUT_FILENO,buf,strlen(buf));
	
	sprintf(buf,"Fibonacci Process Created\n");
	write(STDOUT_FILENO,buf,strlen(buf));
	
	sprintf(buf,"Integration Process Created\n");
	write(STDOUT_FILENO,buf,strlen(buf));
	
	free(buf);
	
	if ( ( children[0] = fork() ) == 0 ){ // heapSort
		
		int* rand_arr;
		buf = malloc(sizeof(char)*200);
		buf_max = 200;
		rand_arr = malloc(sizeof(int)*m);
		srand(time(0));
		
		for (i = 0; i < m; ++i) rand_arr[i] = rand()%100;
		
		buf = strcat(buf,"   HeapSort Process Started\n   Random numbers generated:\n   ");
		write(STDOUT_FILENO,buf,strlen(buf));
		
		buf = strcpy(buf,"");
		
		// gen array of random ints...
		for (i = 0; i < m; ++i) {
			char* tmp;
			tmp = malloc( sizeof(char)*100 );
			sprintf(tmp,"%4d ",rand_arr[i]);
			
			if( ( strlen(tmp) + strlen(buf) + 2) > buf_max ){
				buf_max = buf_max*2;
				buf = realloc(buf,buf_max*2);
			}
			
			buf = strcat(buf,tmp);
			free(tmp);
		}
		
		buf = strcat(buf,"\n");
		write(STDOUT_FILENO,buf,strlen(buf));
		
		heapSort(rand_arr,m); // sort array
		
		// print the sorted array
		strcpy(buf,"   Sorted Sequence:\n   ");
		for (i = 0; i < m; ++i) {
			char* tmp;
			tmp = malloc( sizeof(char)*100 );
			sprintf(tmp,"%4d ",rand_arr[i]);
			
			if( ( strlen(tmp) + strlen(buf) + 2) > buf_max ){
				
				buf_max = buf_max*2;
				buf = realloc(buf,buf_max*2);
				
			}
			
			buf = strcat(buf,tmp);
			free(tmp);
		}
		
		buf = strcat(buf,"\n   HeapSort Process Exits\n");
		write(STDOUT_FILENO,buf,strlen(buf));
		
		// clean up
		free(buf);
		
		return 0;
		
	} else if ( ( children[1] = fork() ) == 0 ) { // fibonacci
		
		char* tmp;
		tmp = malloc(sizeof(char)*100);
		buf = malloc(sizeof(char)*200);
		buf_max=200;

		buf = strcat(buf,"      Fibonacci Process Started\n      ");
		sprintf(tmp,"Input number: %d\n",n);
		buf = strcat(buf,tmp);
		write(STDOUT_FILENO,buf,strlen(buf));
		buf = strcpy(buf,"");
		
		long returnval;
		returnval = fibonacci(n);
		sprintf(buf,"      Fibonacci Number f(%d) is (%ld)\n",n,returnval);
		write(STDOUT_FILENO,buf,strlen(buf));
		
		buf = strcpy(buf, "      Fibonacci process exits\n");
		write(STDOUT_FILENO,buf,strlen(buf));
		
		free(tmp);
		free(buf);
		
		return 0;
		
	} else if ( ( children[2] = fork() ) == 0 ) { // buffons		

		srand(time(0));
		int L,G,t;
		float d,a,b;
		d = a = 0;
		b = t = 0.0;
		L = G = 1;
		char* tmp;
		tmp = malloc(sizeof(char)*100);
		buf = malloc(sizeof(char)*200);
		buf_max = 200;
		
		buf = strcat(buf, "         Buffon's Needle Process Started\n");
		sprintf(tmp,"         Input Number %d\n", r);
		strcat(buf,tmp);
		write(STDOUT_FILENO,buf,strlen(buf));
		
		buf = strcpy(buf,"");
				
		for( i = 0; i < r; ++i ){
		
			// random float [0,1)
			d = a = b = 0;
			d = (float)rand()/(float)RAND_MAX; // will always be [0.1)
			
			// random float [0,2pi)
			a =	(float)rand()/(float)RAND_MAX;
			a = a + (float)(rand()%6);
			
			// a + rand%6 gives [0,6) 2pi ~~ 6.3
			while( a > 2*PI ){ // ensure a is [0,2pi)
			
				a =	(float)rand()/(float)RAND_MAX;
				a = a + (float)(rand()%6);
				
			}
			
			b = d + (float)L * sinf(a);
			
			if(b < 0 || b > G ) ++t; // hit
			
		}
		
		sprintf(buf,"         Estimated Probability is %5f\n",(float)t/(float)r);
		write(STDOUT_FILENO,buf,strlen(buf));
		
		sprintf(buf,"         Buffon's Needle Process Exits\n");
		write(STDOUT_FILENO,buf,strlen(buf));
		
		// clean up
		free(tmp);
		free(buf);
		
		return 0;
		
	} else if ( ( children[3] = fork() ) == 0 ) { // integration
		
		srand(time(0));
		char* tmp;
		float a,b;
		int t;
		tmp = malloc(sizeof(char)*100);
		buf = malloc(sizeof(char)*200);
		a = b = 0.0;
		t = 0;
		
		buf = strcat(buf,"            Integration Process Started\n");
		sprintf(tmp,"            Input Number %d\n",s);
		buf = strcat(buf,tmp);
		write(STDOUT_FILENO,buf,strlen(buf));
		
		for( i = 0; i < s; ++i ){
			
			b = (float)rand()/(float)RAND_MAX; // always [0.1)
			a = (float)rand()/(float)RAND_MAX;
			a = a+rand()%3;
			while( a > PI ){ // ensure a is [0,pi)
			
				a = (float)rand()/(float)RAND_MAX;
				a = a + (float)(rand()%3);
				
			}
			
			if ( b <= sinf(a) ) ++t; // hit
		}
		
		sprintf(buf,"            Total Hits %d\n",t);
		write(STDOUT_FILENO,buf,strlen(buf));
		
		sprintf(buf,"            Estimated Area is %f\n",((float)t/(float)s)*PI);
		write(STDOUT_FILENO,buf,strlen(buf));
		
		sprintf(buf,"            Integration Process Exits\n");
		write(STDOUT_FILENO,buf,strlen(buf));
		
		// clean up
		free(buf);
		free(tmp);
		
		return 0;
		
	} else { // parent stuff

		int bool;
		bool = 0;
		
		while( wait(0) > 0 ) { // while our children do all the work...
			
			if(bool==0){
			
				buf = "Main Process Waits\n";
				write(STDOUT_FILENO,buf,strlen(buf));
				bool=1;
			
			}
			
		}
		
	buf = "Parent Process Exits\n";
	write(STDOUT_FILENO,buf,strlen(buf));
	
	// clean up
	// free(children);
	
	return 0;
	
	}	
}

// -----------------------------------------------------------
// FUNCTION  heapSort
//     Sorts the given array (heap) using the heapsort
//	algorithm.
// PARAMETER USAGE :
//    int* heap: The array to sort
//    int size: The size of the array to sort
// FUNCTION CALLED :
//    heapDown()
// -----------------------------------------------------------
void heapSort(int* heap, int size){
	int start,end;
	for ( start = (size-2)/2; start >= 0; --start ){

		heapDown(heap, start, size);

	}

	for ( end = size-1; end > 0; --end ) {

		SWAP(heap[end],heap[0]);
		heapDown(heap,0,end);

	}

}

// -----------------------------------------------------------
// FUNCTION  heapDown
//     Rebuilds the tree after an insertion
// PARAMETER USAGE :
//    int* heap: The array being heapSorted
//    int start: The index to begin building at
//    int size: The index to stop building at
// FUNCTION CALLED :
//
// -----------------------------------------------------------
void heapDown( int* heap, int start, int end){
	int root = start;

	while ( root*2+1 < end ) {
		int child = 2*root+1;

		if ( ( child + 1 < end) && IS_LESS(heap[child], heap[child+1] ) ){

			child += 1;

		}

		if( IS_LESS(heap[root], heap[child] ) ){

			SWAP( heap[child], heap[root] );
			root = child;

		} else return;

	}

}


// -----------------------------------------------------------
// FUNCTION fibonacci
//     Generates the nth fibonacci number recursively.
// PARAMETER USAGE :
//    int n: The digit in the fibonacci sequence to generate.
// FUNCTION CALLED :
//
// -----------------------------------------------------------
long fibonacci(long n)
{
    if (n < 2)
      return n;
    else 
      return (fibonacci(n - 1) + fibonacci(n - 2));
}

#endif