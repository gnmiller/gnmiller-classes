/**
 * Greg Miller -- gnmiller
 * CS4411 Spring 15
 * Project 2
 * 
 * Due: 2/3/15 09:35
 * Submitted: 2/4/15 00:05 
 * 
 * Slip Days used: 1
 * Slip Days Remaining: 4
 * 
 * File: monitor.cc
 * 
 * Contains the interfaces for Monitor and Condition Variables.
 */

#include <pthread.h>
#include <semaphore.h>
class Condition {
	int   nWaiters;
	sem_t waiterSem;
	public:
		Condition();
		int  Signal(sem_t *signallerSem, int *nSignallers);
		int  Wait(sem_t *monMutex, sem_t *signallerSem, int *nSignallers);
};

class Monitor{
	public:
		sem_t *mutex;
		int   *nSignallers;
		sem_t *signallerSem;
		Monitor();
		void Acquire();
		void Release();
};
