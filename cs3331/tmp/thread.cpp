#ifndef GNMILLER_THREAD_CPP
#define GNMILLER_THREAD_CPP

#include "thread.h"
#include "ThreadClass.h"

#include <iostream>
#include <string.h>
#include <stdio.h>

Semaphore *sem_one;
Semaphore *sem_two;
Semaphore *sem_three;

oneThread::oneThread(){};

void oneThread::ThreadFunc(){
		Thread::ThreadFunc();
		for(int i = 0;i < 10;++i){
			sem_one->Wait();
			cout << "1";
			sem_two->Signal();
			Thread::Delay();
		}
};

twoThread::twoThread(){};

void twoThread::ThreadFunc(){
		Thread::ThreadFunc();
		int sw = 0;
		for(int i = 0;i < 10;++i){
			sem_two->Wait();
			cout << "2";
			if(sw == 0){
				sw = 1;
				sem_three->Signal();
			} else {
				sw = 0;
				sem_one->Signal();
			}
			Thread::Delay();
		}
		exit(0);
};

threeThread::threeThread(){};

void threeThread::ThreadFunc(){
		Thread::ThreadFunc();
		for(int i = 0;i < 10;++i){
			sem_three->Wait();
			cout << "33";
			sem_two->Signal();
			Thread::Delay();
		}
};

#endif