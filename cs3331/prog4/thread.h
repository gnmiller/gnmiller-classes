// ----------------------------------------------------------- 
// NAME : Greg Miller                        User ID: gnmiller
// DUE DATE : 11/14/14                                      
// PROGRAM 4                                        
// FILE NAME : thread.h
// PROGRAM PURPOSE :                                           
//    Function and class headers for the various threads used
//		in the application.
// ----------------------------------------------------------- 

#ifndef GNMILLER_THREAD_H
#define GNMILLER_THREAD_H

#include "ThreadClass.h"

#define ELF_MAX 3
#define ELF_DEFAULT 7
#define DEER_DEFAULT 9
#define DELIVERY_DEFAULT 5

/* Shared variables */

/* semaphore */
extern Semaphore *Sleeping; // 0
extern Semaphore *Reindeer; // r
extern Semaphore *Elf; // 3
extern Semaphore *Sleigh; // 0
extern Semaphore *Question; // 0
extern Semaphore *Answered; // 0
extern Semaphore *Flying; // 0
extern Semaphore *Waiting_Deer; // 0
extern Semaphore *Hitched; // 0
extern Semaphore *Unhitch; // 0

/* mutex */
extern Mutex *Reindeer_Lock;
extern Mutex *Elf_Lock;
extern Mutex *Print_Lock;

/* other shared */
extern int Reindeer_Count;
extern int Elf_Count;
extern int *waiting_elves; // buffer

/* function headers */
void my_printf(char *s);
void AskQuestion(int i);
void ReindeerBack(int i, int r);
void WaitOthers();
void WaitSleigh();
void FlyOff();
void Sleep();

/* elf class headers */
class ElfThread : public Thread {
	public:
		ElfThread(char *n, int i);
	private:
		void ThreadFunc();
		char *name;
		int i;
};

/* reindeer class def */
class ReindeerThread : public Thread {
	public:
		ReindeerThread(char *n, int i, int r);
	private:
		void ThreadFunc();
		char *name;
		int i,r;
};

/* santa class def */
class SantaThread : public Thread {
	public:
		SantaThread(char *n, int max, int r);
	private:
		void ThreadFunc();
		char *name;
		int delivery,max_delivery,r;
};
#endif