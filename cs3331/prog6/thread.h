/*
 * Name: Greg Miller		User ID: gnmiller
 * Due: 12/12/14
 * Program 5
 * thread.h
 * 	Contains headers and class definitions for the following:
 * 		Classes
 * 			ThreadRowReader
 * 			ThreadColReader
 * 			ThreadProcessor
 * 		Functions
 * 			my_printf()
 * 		Data
 * 			dataA, dataB, output
 * 			p
 * 			A_X,A_Y,B_X,B_Y
 * 			Print_Lock
 */

#ifndef GNMILLER_THREAD_H
#define GNMILLER_THREAD_H

#include "ThreadClass.h"

/* an alias for SynOneToOneChannel */
typedef SynOneToOneChannel SynChannel;

class ThreadProcessor : public Thread{
	public:
		ThreadProcessor( char*, int, int, int );
		ThreadProcessor();
		
		SynChannel *left;
		void setLeft( SynChannel*);
		//SynChannel *getLeft();
		
		SynChannel *right;
		void setRight( SynChannel*);
		//SynChannel *getRight();
		
		SynChannel *down;
		void setDown( SynChannel*);
		//SynChannel *getDown();
		
		SynChannel *up;
		void setUp( SynChannel*);
		//SynChannel *getUp();
		
		//char* hasNeighbors();
		
		int threadID;
		int hasU,hasL,hasR,hasD;
	private:	
		void ThreadFunc();
		int i;
		int j;
		int t_r;
		int t_c;
		int storage;
};

class  ThreadColReader : public Thread{
	public:
		ThreadColReader( char*, int, int );
		ThreadColReader();
		
		int threadID;
	private:
		void ThreadFunc();
		int index;
};

class ThreadRowReader : public Thread{
	public:
		ThreadRowReader( char*, int, int );
		ThreadRowReader();
		
		int threadID;
	private:
		void ThreadFunc();
		int index;
};

/* printing stuff */
extern Mutex *Print_Lock;
void my_printf(char *);

/* global data */
extern ThreadProcessor ***p;
extern int **output;
extern int **dataA;
extern int **dataB;
extern int A_X;
extern int A_Y;
extern int B_X;
extern int B_Y;

#endif