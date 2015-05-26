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
 * Contains the implementation of the interfaces for Monitor and 
 * Condition Variable as specified in monitor.h that has Hoare semantics.
 */

#include <pthread.h>
#include <errno.h>
#include "monitor.h" 

Condition::Condition()
{
	sem_init( &waiterSem, 0, 0 );  // init waiterSem to be shared (arg2) and open initially
	nWaiters = 0;
};

/**
 * Condition::Signal( sem_t signallerSem, int nSignallers )
 *
 * signallerSem: The semaphore used by the monitor to block signallers
 * nSignallers: The number of processes waiting on signallerSem
 *
 * Signals the semaphore signallerSem releasing
 * a process waiting on it. If there is no process waiting
 * the function returns immediately.
 * 
 * In the event of an error, errno is returned. Otherwise
 * the function returns 0.
 */
int  Condition::Signal(sem_t *signallerSem, int *nSignallers)
{
	if( nWaiters > 0 ) // someone is waiting
	{
		++nSignallers;
		if( sem_post( &waiterSem ) != 0 ) return errno;
		if( sem_wait( signallerSem ) != 0 ) return errno;
		--nSignallers;
	}
	return 0;
};

/**
 * Condition::Wait( sem_t monMutex, sem_t signallerSem, int nSignallers )
 *
 * monMutex: Mutex used to allow or disallow access to the monitor.
 * signallerSem: The semaphore used by the monitor to block signallers
 * nSignallers: The number of processes waiting on signallerSems
 * 
 * If there is no processes waiting after being signalled the process
 * yields to one of them. If there is no process waiting then the process 
 * yields the monitor lock, the process then waits for no other processes to be
 * waiting on this CV (waiterSem). Once this is the case the process will be
 * awaken and return.
 *
 * In the event of errors the errno is return. If the function exits correctly
 * 0 will be returned.
 */
int  Condition::Wait(sem_t *monMutex, sem_t *signallerSem, int *nSignallers){
	++nWaiters;
	if( nSignallers > 0 )
	{
		if ( sem_post( signallerSem ) != 0 ) return errno;
	}
	else
	{
		if ( sem_post( monMutex ) != 0 ) return errno;
	}
	if( sem_wait( &waiterSem ) != 0 ) return errno;
	return 0;
};

/* Monitor default constructor */
Monitor::Monitor(){
    signallerSem = new( sem_t );
    mutex = new( sem_t );
    sem_init( signallerSem, 0, 0 );
    sem_init( mutex, 0, 1 );
    nSignallers = new( int );
	*nSignallers=0;
};

/* Acquire the lock for the monitor, ensuring no other process may enter */
void Monitor::Acquire(){
	sem_wait(mutex);
};

/* Release the lock, allowing any process to enter the monitor */
void Monitor::Release(){
    if ( ( *nSignallers ) > 0 ) sem_post( signallerSem ); //-- Free any signallers 
    else sem_post(mutex);
};