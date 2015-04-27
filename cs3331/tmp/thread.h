#ifndef GNMILLER_THREAD_H
#define GNMILLER_THREAD_H

#include "ThreadClass.h"

extern Semaphore *sem_one;
extern Semaphore *sem_two;
extern Semaphore *sem_three;

class oneThread : public Thread {
	public:
		oneThread();
	private:
		void ThreadFunc();
};

class twoThread : public Thread {
	public:
		twoThread();
	private:
		void ThreadFunc();
};

class threeThread : public Thread {
	public:
		threeThread();
	private:
		void ThreadFunc();
};

#endif