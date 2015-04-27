// ----------------------------------------------------------- 
// NAME : Greg Miller                        User ID: gnmiller
// DUE DATE : 11/14/14                                      
// PROGRAM 4                                        
// FILE NAME : thread.cpp
// PROGRAM PURPOSE :                                           
//    Class definitions for threads.
// -----------------------------------------------------------

#ifndef GNMILLER_THREAD_CPP
#define GNMILLER_THREAD_CPP

#include "ThreadClass.h"
#include "thread.h"
#include "stdio.h"

/* Begin Elf definitions */

// ----------------------------------------------------------- 
//	FUNCTION  ElfThread::ElfThread()                         
//		Constructor for ElfThread's.                            
//	PARAMETER USAGE :                                           
//		n: A string representing the thread's name.
//		i: An integer representing which thread we are [0-n)
//	FUNCTION CALLED :                                           
//		n/a
// ----------------------------------------------------------- 
ElfThread::ElfThread(char *n, int i) : i(i), name(n) { /* empty constructor */ };


// ----------------------------------------------------------- 
//	FUNCTION  ElfThread::ThreadFunc()                          
//		The "main" method of ElfThread's. Causes the Elves to
//	try to ask a question when there are 3, then wait for the
//	answer.
//	PARAMETER USAGE :                                           
//		n/a
//	FUNCTION CALLED :                                           
//		Semaphore::Wait()
//		ElfThread::AskQuestion()
//		my_printf()
// ----------------------------------------------------------- 
void ElfThread::ThreadFunc(){	
	char buf[100];
	sprintf(buf, "         Elf %d starts.", i);
	my_printf(buf);
	for(;;){
		Elf->Wait(); // wait for elves that may be asking already
		Thread::Delay();
		AskQuestion(i); // ask our question
		Thread::Delay();
	}	
};

/* End Elf definitions */
/* ------------------------------------------------------------------------- */
/* Begin Reindeer definitions */

// ----------------------------------------------------------- 
//	FUNCTION ReindeerThread::ReindeerThread
//		ReindeerThread constructor                            
//	PARAMETER USAGE :                                           
//		n: The name of this thread
//		i: The integer representation of this thread.
//	FUNCTION CALLED :                                           
//		n/a
// ----------------------------------------------------------- 
ReindeerThread::ReindeerThread(char *n, int i, int r) : name(n), i(i), r(r){
	/* empty constructor */ 
};

// ----------------------------------------------------------- 
//	FUNCTION ReindeerThread::ThreadFunc() 
//		The "main" method for reindeer. Calls the various 
//	other functions, blocking and signalling Santa and other
//	reindeer as needed through calls to other functions.
//	PARAMETER USAGE :                                           
//		n/a               
//	FUNCTION CALLED :                                           
//		ReindeerBack();
//		WaitOthers();
//		WaitSleigh();
//		my_printf()
//		sprintf()
// ----------------------------------------------------------- 
void ReindeerThread::ThreadFunc(){
	char buf[100];
	sprintf(buf, "    Reindeer %d starts.", i);
	my_printf(buf);
	for(;;){
		Reindeer->Wait(); // wait until vacation is over
		Thread::Delay();
		ReindeerBack(i,r); // check in
		WaitOthers(); // wait for the car to fill
		WaitSleigh(); // wait for car to start
		FlyOff(); // driving around
		Thread::Delay();
	}
};

/* End Reindeer definitions */
/* ------------------------------------------------------------------------- */
/* Begin Santa definitions */
// ----------------------------------------------------------- 
//	FUNCTION ReindeerThread::ThreadFunc() 
//		Constructor for Santa
//	PARAMETER USAGE :                                           
//		n/a               
//	FUNCTION CALLED :                                           
//		n/a
// ----------------------------------------------------------- 
SantaThread::SantaThread(char *n, int max, int r) : name(n), delivery(0),
	max_delivery(max), r(r){ /* empty constructor */ };

void SantaThread::ThreadFunc(){
	char buf[100];
	my_printf(" Santa thread begins.");
	while( delivery <= max_delivery ){
		Sleep(); // wait for someone to wake me up
	
		Reindeer_Lock->Lock();
		if( Reindeer_Count == r ){
			Reindeer_Lock->Unlock();
		
			my_printf(" Santa is preparing the sleigh.");
			
			for(int i = 0; i < r; ++i) Sleigh->Signal(); // hitch up some reindeer
			for(int i = 0; i < r; ++i) Hitched->Wait(); // wait for them to hitch up
			
			sprintf(buf," Santa is now delivering toys (%d)!\n",++delivery);
			my_printf(buf);
			
			// delivering some toys
			Thread::Delay();
		
			for(int i = 0; i < r; ++i) Flying->Signal(); // tell the deer theyre free
			for(int i = 0; i < r; ++i) Unhitch->Wait(); // wait for reindeer to leave
			Reindeer_Count = 0; // no reindeer can be unblocked and need this data
			// tell reindeer were ready to deliver again eventually
			for(int i = 0; i < r; ++i) Reindeer->Signal();
		} else {
			Reindeer_Lock->Unlock();
			
			my_printf(" Santa is answering the elf's questions.");
			for(int i = 0; i < ELF_MAX; ++i) Question->Signal(); // listen to elves
			sprintf(buf,
					" Santa has answered the questions posed by elves %d, %d, %d",
					waiting_elves[0],waiting_elves[1],waiting_elves[2]);
			for(int i = 0; i < ELF_MAX; ++i) Answered->Signal();
		}
		// go back to sleep for now
	}
	my_printf("Santa is an old man and has decided to retire!");
	Exit();
};

#endif