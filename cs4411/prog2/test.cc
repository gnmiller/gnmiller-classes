#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include "monitor.h"
#include <pthread.h>
void *p_func( void *v );
void *p_func2( void *v );
void *p_func3( void *v );
int j;
Monitor *Mon;
Condition *cv;
using namespace std;
/* in the current layout in ALL executions i is 0 */
int main( int argc, char **argv )
{
	Mon = new Monitor();
	cv = new Condition();
	int t_c = 1000;
	pthread_t threads[t_c];
	pthread_t waiter;
	j = 0;
	for( int i = 0; i < t_c/2; ++i ) pthread_create( &threads[i], NULL, &p_func, NULL );
	for( int i = (t_c/2); i < t_c; ++i ) pthread_create( &threads[i], NULL, &p_func2, NULL );
	pthread_create( &waiter, NULL, &p_func3, NULL );
	/* this signal is almost always lost */
	//cv->Signal( Mon->signallerSem, Mon->nSignallers );
	for( int i = 0; i < t_c; ++i ) pthread_join( threads[i], NULL );
	/* this signal always arrives */
	cv->Signal( Mon->signallerSem, Mon->nSignallers );
	pthread_join( waiter, NULL );
}

void* p_func( void *v )
{
	Mon->Acquire();
	++j;
	Mon->Release();
}

void* p_func2( void *v )
{
	Mon->Acquire();
	--j;
	Mon->Release();
	/* uncommenting this we can see the signaller yield and in some cases deadlock
	 * as the waiter is not waiting yet */
	//cv->Signal( Mon->signallerSem, Mon->nSignallers );
}

void* p_func3( void *v )
{
	cv->Wait( Mon->mutex, Mon->signallerSem, Mon->nSignallers );
	cout << j << endl;
}

/*
 cv->wait( Mon->mutex, Mon->signallerSem, Mon->nSignallers );
 Mon->Acquire();
 printf("p1\n");
 Mon->Release();
 //
 cv->signal( Mon->signallerSem, Mon->nSignallers );
 Mon->Acquire();
 printf("p2\n");
 Mon->Release();