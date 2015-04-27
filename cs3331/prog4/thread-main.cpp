// ----------------------------------------------------------- 
// NAME : Greg Miller                        User ID: gnmiller
// DUE DATE : 11/14/14                                      
// PROGRAM 4                                        
// FILE NAME : thread-main.cpp
// PROGRAM PURPOSE :                                           
//    Main driver for application. Creates and manages threads
//	and initializes various data values for threads to use.
// ----------------------------------------------------------- 

#ifndef GNMILLER_THREAD_MAIN_CPP
#define GNMILLER_THREAD_MAIN_CPP

#include "ThreadClass.h"
#include "thread.h"
#include <iostream>
#include <stdlib.h>

/* semaphore */
Semaphore *Sleeping;
Semaphore *Reindeer;
Semaphore *Elf;
Semaphore *Sleigh;
Semaphore *Question;
Semaphore *Answered;
Semaphore *Flying;
Semaphore *Waiting_Deer;
Semaphore *Hitched;
Semaphore *Unhitch;

/* mutex */
Mutex *Elf_Lock;
Mutex *Reindeer_Lock;
Mutex *Print_Lock;

/* counters */
int Elf_Count;
int Reindeer_Count;
int *waiting_elves;

int main(int argc, char** argv){
	if( argc != 4 ){
		cerr << "Not enough arguements." << endl;
		cerr << "Usage: prog4 e r t" << endl;
		cerr << "   e: Number of elves." << endl;
		cerr << "   r: Number of reindeer." << endl;
		cerr << "   t: Number of deliviers to make." << endl;
		exit(1);
	}
	
	int r, e, t;
	e = atoi(argv[1]);
	r = atoi(argv[2]);
	t = atoi(argv[3]);
	
	if( e == 0 ) e = ELF_DEFAULT;
	if( r == 0 ) r = DEER_DEFAULT;
	if( t == 0 ) t = DELIVERY_DEFAULT;

	Sleeping = new Semaphore("Sleeping", 0);
	Reindeer = new Semaphore("Reindeer", r);
	Elf = new Semaphore("Elf", 3);
	Sleigh = new Semaphore("Sleigh", 0);
	Question = new Semaphore("Question", 0);
	Answered = new Semaphore("Answered", 0);
	Flying = new Semaphore("Flying", 0);
	Waiting_Deer = new Semaphore("W_Deer", 0);
	Hitched = new Semaphore("Hitched", 0);
	Unhitch = new Semaphore("Unhitch", 0);
	
	Reindeer_Lock = new Mutex("R_Lock");
	Elf_Lock = new Mutex("E_Lock");
	Print_Lock = new Mutex("P_Lock");
	
	Reindeer_Count = 0;
	Elf_Count = 0;
	waiting_elves = new int[3];
	int i;
	
	ReindeerThread *deer[r];
	for(i = 0; i < r; ++i) { deer[i] = new ReindeerThread("Reindeer", i, r); }
	
	ElfThread *elf[e];
	for(i = 0; i < e; ++i) { elf[i] = new ElfThread("Elf", i); }
	
	SantaThread *santa = new SantaThread("santa", t, r);
	
	for(i = 0; i < r; ++i) { deer[i]->Begin(); }
	for(i = 0; i < e; ++i) { elf[i]->Begin(); }
	santa->Begin();
	santa->Join();
	exit(0);
}
#endif