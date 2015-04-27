// ---------------------------------------------------------------------------- 
// NAME : Gre Miller										User ID: gnmiller 
// DUE DATE : 10/31/14                                       
// PROGRAM ASSIGNMENT 3                                        
// FILE NAME : thread.h           
// PROGRAM PURPOSE :                                           
//    Contains declarations for classes and global variables for sharing among
//	threads.
// ----------------------------------------------------------------------------

#ifndef GNMILLER_THREAD_H
#define GNMILLER_THREAD_H

#include "ThreadClass.h"

extern Semaphore *wall, *f_wall;
extern Mutex *s_lock;
extern int s_count, f_count;
extern int *x; // the array to search
extern int *w; // the array to use for finding the max (0s, and 1s)

/* initiliaze the array w to all 1s */
class initializeThread : public Thread {

	public:
		initializeThread(int i, int m);
	private:
		void ThreadFunc();
		int my_i,max;
};

/* iterate over pairs of ints in the array and set value in w accordingly */
class seekerThread : public Thread {

	public:
		seekerThread(int i, int j, int m, int t);
	private:
		void ThreadFunc();
		int my_i, my_j, max, total;
		
};

/* iterate over the list and check if i am the max */
class greatestThread : public Thread {

	public:
		greatestThread(int i);
	private:
		void ThreadFunc();
		int my_i;
};

#endif