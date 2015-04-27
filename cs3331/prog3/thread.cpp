// ---------------------------------------------------------------------------- 
// NAME : Gre Miller										User ID: gnmiller 
// DUE DATE : 10/31/14                                       
// PROGRAM ASSIGNMENT 3                                        
// FILE NAME : thread.cpp            
// PROGRAM PURPOSE :                                           
//    Function definitions for classes (constructors, etc).
// ----------------------------------------------------------------------------

#ifndef GNMILLER_THREAD_CPP
#define GNMILLER_THREAD_CPP

#include "thread.h"
#include "ThreadClass.h"

#include <iostream>
#include <string.h>
#include <stdio.h>


Semaphore *wall;
Semaphore *f_wall;
Mutex *s_lock;
int s_count, f_count;
int *x; // the array to search
int *w; // the array to use for finding the max (0s, and 1s)

/* init thread definitions */

// ---------------------------------------------------------------------------- 
// FUNCTION  initThread::initThread(int i, int m)
//     Constructor for initThread                            
// PARAMETER USAGE :                                           
//		i - The index of the array (w) that this thread will set
//		m - The number of init threads created. Used for signalling the next
//				"team"
// FUNCTION CALLED :                                           
//		n/a
// ----------------------------------------------------------------------------
initializeThread::initializeThread(int i, int m){ 
	my_i = i;
	max = m;
};

// ---------------------------------------------------------------------------- 
// FUNCTION  initThread::ThreadFunc()
//     Performs the initializing step of the search.
// PARAMETER USAGE :                                           
//		n/a
// FUNCTION CALLED :                                           
//		Thread::ThreadFunc()
// ----------------------------------------------------------------------------
void initializeThread::ThreadFunc(){
	Thread::ThreadFunc();
	
	w[my_i] = 1;
	Exit();
};

/* end init definitions */

/* begin seeker definitions */

// ---------------------------------------------------------------------------- 
// FUNCTION  seekerThread::seekerThread(int i, int j, int m)
//		Constructor for seekerThread.
// PARAMETER USAGE :                                           
//		i - The first index to compare.
//		j - The second index to compare.
//		m - The maximum number of seeker threads to be created. Used to signal
//				the final team of threads.
// FUNCTION CALLED :                                           
//		n/a
// ----------------------------------------------------------------------------
seekerThread::seekerThread(int i, int j, int m, int t){
		my_i = i;
		my_j = j;
		max = m;
		total = t;
};

// ---------------------------------------------------------------------------- 
// FUNCTION  seekerThread::ThreadFunc()
//		Perform the comparison step of the search.
// PARAMETER USAGE :                                           
//		n/a
// FUNCTION CALLED :                                           
//		Thread::ThreadFunc()
// ----------------------------------------------------------------------------
void seekerThread::ThreadFunc(){
	Thread::ThreadFunc();
	
	char buf[100];
	if(x[my_i] < x[my_j]){
		w[my_i] = 0;
		sprintf(buf, "Thread(%d,%d) compares x[%d] = %d and x[%d] = %d and writes 0 into w[%d]\n",
				my_i, my_j, my_i, x[my_i], my_j, x[my_j], my_i);
		write(STDOUT_FILENO, buf, strlen(buf));
	}
	else {
		w[my_j] = 0;
		sprintf(buf, "Thread(%d,%d) compares x[%d] = %d and x[%d] = %d and writes 0 into w[%d]\n",
				my_i, my_j, my_i, x[my_i], my_j, x[my_j], my_j);
		write(STDOUT_FILENO, buf, strlen(buf));
	}
	
	/* enter the critical section */
	s_lock->Lock();
	++s_count;
	/* are we last? if we are tell the next guys (all of them) to go */
	if(s_count == max) {
		for(int i = 0; i < total; ++i) { 
			wall->Signal();
		}
	}
	s_lock->Unlock();
	/* exit the critical section */
	Exit();
};

/* end seeker definitions */

/* begin greatest definitions */

// ---------------------------------------------------------------------------- 
// FUNCTION  greatestThread::greatestThread(int i)
//		Constructor for the final set of threads.
// PARAMETER USAGE :                                           
//		i - The first index to compare.
// FUNCTION CALLED :                                           
//		n/a
// ---------------------------------------------------------------------------
greatestThread::greatestThread(int i){
	my_i = i;
};

// ---------------------------------------------------------------------------- 
// FUNCTION  greatestThread::ThreadFunc()
//		Perform the final step (finding the max).
// PARAMETER USAGE :                                           
//		n/a
// FUNCTION CALLED :                                           
//		Thread::ThreadFunc()
// ---------------------------------------------------------------------------
void greatestThread::ThreadFunc(){
	Thread::ThreadFunc();
	
	/* wait until the seekers finish */
	wall->Wait();
	if(w[my_i] == 1){
		char buf[100];
		sprintf(buf, "Maximum			  = %d\nIndex			  = %d\n",
				x[my_i], my_i);
		write(STDOUT_FILENO, buf, strlen(buf));
		
		/* we know that all seekers are done (otherwise we would be blocked */
		f_wall->Signal();
	}
	Exit();
};

#endif