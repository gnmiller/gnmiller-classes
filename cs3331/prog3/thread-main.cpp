// ---------------------------------------------------------------------------- 
// NAME : Gre Miller										User ID: gnmiller 
// DUE DATE : 10/31/14                                       
// PROGRAM ASSIGNMENT 3                                        
// FILE NAME : thread-main.cpp            
// PROGRAM PURPOSE :                                           
//    Main driver for the threaded application. Allocated memory, manages
//	threads and prints (some) data.
// ----------------------------------------------------------------------------

#ifndef GNMILLER_THREAD_MAIN_CPP
#define GNMILLER_THREAD_MAIN_CPP

#include "thread.h"
#include "ThreadClass.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char** argv){
	
	wall = new Semaphore("wall", 0);
	f_wall = new Semaphore("f_wall", 0);
	s_lock = new Mutex("s_lock");
	
	int num_seekers = 0, size = atoi(argv[1]);
	x = new int[ size ];
	w = new int[ size ];
	char buf[100], buf2[100];
	
	sprintf(buf, "Number of input values	  = %d\n", size);
	write(STDOUT_FILENO, buf, strlen(buf));
	
	sprintf(buf, "Input values		x = [");
	/* 
	 * parse command line arguments
	 * argv0 = proc name, argv0 = # args argv = x[0] ...
	 */
	for(int i = 0; i < size; ++i){
		x[i] = atoi( argv[ 2+i ] );
		sprintf(buf2, " %d ", x[i]);
		strcat(buf, buf2);
	} strcat(buf, "] \n");
	write(STDOUT_FILENO, buf, strlen(buf));
	
	/* these need n iterations but we need less threads for comparing */
	initializeThread **init = new initializeThread*[ size ];
	greatestThread **greatest = new greatestThread*[ size ];

	/* count the # of seekers we need */
	int offset = 1;
	for(int i = 0; i < size-1; ++i){
		for(int j = offset; j < size; ++j){
				++num_seekers;
		}
		++offset;
	}
	seekerThread **seeker = new seekerThread*[ num_seekers ];
	
	s_count = 0;
	for(int i = 0; i < size; ++i){
		init[i] = new initializeThread(i, size);
		init[i]->Begin();
		greatest[i] = new greatestThread(i);
		greatest[i]->Begin();
	}
	
	
	/* wait for all the init threads so we can print out w safely */
	for(int i = 0; i < size; ++i) init[i]->Join();
	sprintf(buf, "After init step		w = [");
	for(int i = 0; i < size; ++i){
		sprintf(buf2, " %d ", w[i]);
		strcat(buf, buf2);
	} strcat(buf, "] \n");
	write(STDOUT_FILENO, buf, strlen(buf));
	
	offset = 1;
	int test = 0;
	for(int i = 0; i < size-1; ++i){
		for(int j = offset; j < size; ++j){
			seeker[i] = new seekerThread(i, j, num_seekers, size);
			seeker[i]->Begin();
		} test = i;
		++offset;
	}
	
	/* some of the seekers become NULL eventually by this point? */
	//for(int i = 0; i < num_seekers; ++i) seeker[i]->Join();

	f_wall->Wait();
	sprintf(buf, "After final step	w = [");
	for(int i = 0; i < size; ++i){
		sprintf(buf2, " %d ", w[i]);
		strcat(buf, buf2);
	} strcat(buf, "] \n");
 	write(STDOUT_FILENO, buf, strlen(buf));	
	
	Exit();
}

#endif