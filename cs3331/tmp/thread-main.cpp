#ifndef GNMILLER_THREAD_MAIN_CPP
#define GNMILLER_THREAD_MAIN_CPP

#include "thread.h"
#include "ThreadClass.h"

#include <iostream>
#include <string.h>
#include <stdio.h>

int main(){
	sem_one = new Semaphore("1",1);
	sem_two = new Semaphore("2",0);
	sem_three = new Semaphore("3",0);
	
	oneThread *first = new oneThread();
	twoThread *second = new twoThread();
	threeThread *third = new threeThread();
	
	first->Begin();
	second->Begin();
	third->Begin();
	
	first->Join();
	second->Join();
	third->Join();
}

#endif