// ----------------------------------------------------------- 
// NAME : Greg Miller                        User ID: gnmiller
// DUE DATE : 11/14/14                                      
// PROGRAM 4                                        
// FILE NAME : thread-support.cpp
// PROGRAM PURPOSE :                                           
//    Function definitions threads.
// -----------------------------------------------------------

#ifndef GNMILLER_THREAD_SUPPORT_CPP
#define GNMILLER_THREAD_SUPPORT_CPP

#include "ThreadClass.h"
#include "thread.h"
#include "stdio.h"

// ----------------------------------------------------------- 
//	FUNCTION my_print:                          
//		A helper function for ensuring that printing is atomic
//	withouth the use of the write() system call.           
//	PARAMETER USAGE :                                           
//		s: The string to print to stdout
//	FUNCTION CALLED :                                           
// 		Mutex::Lock()
// 		Mutex::Unlock()
// ----------------------------------------------------------- 
void my_printf(char *s){
	Print_Lock->Lock();
		cout << s << endl;
	Print_Lock->Unlock();
}

// ----------------------------------------------------------- 
//	FUNCTION AskQuestion()                          
//		The elves (when 3 are present) try to wake Santa. They
//	block until Santa listens to their problems. Then block
//	again until Santa has answered their questions.
//	PARAMETER USAGE :                                           
//		n/a              
//	FUNCTION CALLED :                                           
//		Mutex::Lock()
//		Mutex::Unlock()
//		Semaphore::Wait()
//		Semaphore::Signal()
//		Thread::Delay()
//		my_printf()
//		sprintf()
// ----------------------------------------------------------- 
void AskQuestion(int i){	
	char buf[100];
	sprintf(buf, "        Elf %d has a question.", i);
	my_printf(buf);
	
	Elf_Lock->Lock();
		waiting_elves[Elf_Count] = i; // produce
		++Elf_Count;
		if( Elf_Count == 3 ){
			sprintf(buf, "        Elves %d, %d, %d try to wake up Santa.",
					waiting_elves[0], waiting_elves[1], waiting_elves[2]);
			my_printf(buf);
			Sleeping->Signal();
		}
	Elf_Lock->Unlock();
	
	Question->Wait(); // wait for santa to listen
	Answered->Wait(); // wait for santa to respond
	waiting_elves[0] = 0;
	waiting_elves[1] = 0;
	waiting_elves[2] = 0;
	
	Elf_Lock->Lock();
		--Elf_Count;
		if( Elf_Count == 0 ){
			sprintf(buf, "        Elves %d, %d, %d return to work.",
					waiting_elves[0], waiting_elves[1], waiting_elves[2]);
			my_printf(buf);
			for(int j = 0; j < 3; ++j) Elf->Signal(); // let the next 3 elves in
		}
	Elf_Lock->Unlock();	
};



// ----------------------------------------------------------- 
//	FUNCTION ReindeerBack() 
//		Each reindeer checks in to let us know that he is here
//	and ready to be attached to the sleigh. When r deer have 
//	arrived they wake up Santa to deliver toys.
//	PARAMETER USAGE :                                           
//		n/a               
//	FUNCTION CALLED :                                           
//		Mutex::Lock()
//		Mutex::Unlock()
//		my_printf()
//		sprintf()
// ----------------------------------------------------------- 
void ReindeerBack(int i, int r){
	char buf[100];
	// this does not need to be in the CS below as buf is unique and my_printf 
	// an atomic print
	sprintf(buf, "    Reindeer %d is back from vacation.", i);
	my_printf(buf);
	Reindeer_Lock->Lock();
		++Reindeer_Count;
		if( Reindeer_Count == r ){
			// tell all the reindeer that were ready to go
			for(int j = 0; j < r; ++j) Waiting_Deer->Signal();
			// wake santa
			sprintf(buf,
					"    Reindeer %d was the last back and is waking up Santa!",
					i);
			my_printf(buf);
			Sleeping->Signal();
		}
	Reindeer_Lock->Unlock();
};

// ----------------------------------------------------------- 
//	FUNCTION WaitOthers()
//		Wait for the other 9 deer to arrive.
//	PARAMETER USAGE :                                           
//		n/a               
//	FUNCTION CALLED :                                           
//		Semaphore::Wait()
// ----------------------------------------------------------- 
void WaitOthers(){
	// the rth deer will signal this semaphore r times
	// releasing all deer including himself
	Waiting_Deer->Wait();
};

// ----------------------------------------------------------- 
//	FUNCTION WaitSleigh()
//		Wait for Santa to hitch all r of us up and then start
//	delivering toys.
//	PARAMETER USAGE :                                           
//		n/a               
//	FUNCTION CALLED :                                           
//		Semaphore::Wait()
//		Semaphore::Signal()
// -----------------------------------------------------------
void WaitSleigh(){
	Sleigh->Wait(); // wait for santa to hitch us up
	Hitched->Signal(); // tell santa that were hitched up
};

// ----------------------------------------------------------- 
//	FUNCTION FlyOff()
//		Deliver some toys.
//	PARAMETER USAGE :                                           
//		n/a               
//	FUNCTION CALLED :                                           
//		Semaphore::Wait()
//		Semaphore::Signal()
// -----------------------------------------------------------
void FlyOff(){
	Flying->Wait(); // wait to get back
	Unhitch->Signal(); // tell santa were going back on vacation
};

// ----------------------------------------------------------- 
//	FUNCTION FlyOff()
//		Deliver some toys.
//	PARAMETER USAGE :                                           
//		n/a               
//	FUNCTION CALLED :                                           
//		Semaphore::Signal()
// -----------------------------------------------------------
void Sleep(){
	Sleeping->Wait(); // wait to be awoken
}
#endif