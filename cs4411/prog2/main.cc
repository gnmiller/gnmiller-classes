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
 * File: main.cc
 * 
 * Driver for refstats.
 * Parses the log files specified on the command line and performs
 * a DNS lookup to retrieve the FQDN of the host specified.
 * 
 * It is invoked as:
 * refstats -b numlines -N cachesize -d filedelay -D threaddelay -C file
 * -b is the number of lines to read from each file
 * -N is the max lines in the cache
 * -d is the time the reader sleeps after each iteration in ms
 * -D is the time the thinkers sleep after each iteration in ms
 * -C is an optional command to print the cache after the threads
 * finish parsing the log and calculating the FQDN's.
 * file specifies any number of files that the application is to parse.
 */


#include <cstdlib>
#include <cstdio>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <list>

/* DNS lookup includes */
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "monitor.h"
#include "helper.h"

/* macro defs */
#define MAX_LINES 1000
#define MIN_LINES 1
#define MAX_LINE_SIZE 33
#define MAX_CACHE 10000
#define MIN_CACHE 1
#define MIN_FILE_D 0
#define MIN_THREAD_D 0
#define MAX_PATH 4097

/* prototypes */
void *ReaderThread( void* );
void *ThinkerThread( void* );
bool compare_ip( const IPNode *first, const IPNode *second );
bool unique_ip( const IPNode *first, const IPNode *second );

/* some global data for threads */
Monitor *Mon;
int bufSize, numlines, cachesize, filedelay, threaddelay;
int FLAG;
pthread_t reader;

/* linked lists for data storage */
using namespace std;
list<Node*> *file_list; // holds file names
list<Node*> *buffer; // holds the IPs read from the file
list<IPNode*> *cache; // holds the cache
list<IPNode*> *data; // holds the IP and the DNS data
list<RNode*> *output; // will hold the outputted data


int main( int argc, char **argv )
{	
	const char *invoke = "Invocation: refstats -b numlines -N cachesize -d filedelay -D threaddelay -C file";
	int flags[5], i, CACHE_FLAG;
	flags[0] = flags[1] = flags[2] = flags[3] = flags[4] = 0;
	bufSize = CACHE_FLAG = FLAG = 0;
	
	Mon = new Monitor();
	
	/* linked lists for data storage */
	file_list = new list<Node*>();
	buffer = new list<Node*>();
	cache = new list<IPNode*>();
	data = new list<IPNode*>();
	output = new list<RNode*>();
	
	/* parse arguments */
	if( argc == 1 || argc < 10 )
	{
		fprintf( stderr, "Invalid arguments\n%s\n", invoke);
		exit( EXIT_FAILURE );
	}
	for( i = 1; i < argc; i+=2 )
	{
		if( !strcmp( argv[i], "-b" ) )
		{
			if( ( atoi( argv[i+1] ) > MAX_LINES 
				|| atoi( argv[i+1] ) < MIN_LINES ) 
				&& flags[0] == 0 ) 
			{
				fprintf( stderr, "Invalid arguments (-b)\n%s\n", invoke);
				exit( EXIT_FAILURE );
			}
			else numlines = atoi( argv[i+1] );
			flags[0] = 1;
		}
		else if( !strcmp( argv[i], "-N" ) )
		{
			
			if( ( atoi( argv[i+1] ) > MAX_CACHE 
				|| atoi( argv[i+1] ) <  MIN_CACHE ) 
				&& flags[1] == 0 )
			{
				fprintf(stderr, "Invalid arguments (-N)\n%s\n", invoke);
				exit( EXIT_FAILURE );
			} 
			else cachesize = atoi( argv[i+1] );
			flags[1] = 1;
		}
		else if( !strcmp( argv[i], "-d" ) )
		{			
			if( atoi( argv[i+1] ) < MIN_THREAD_D 
				&& flags[2] == 0 )
			{
				fprintf(stderr, "Invalid arguments (-d)\n%s\n", invoke);
				exit( EXIT_FAILURE );
			} 
			else threaddelay = atoi( argv[i+1] );
			flags[2] = 1;
		}
		else if( !strcmp( argv[i], "-D" ) )
		{
			if( atoi( argv[i+1] ) < MIN_FILE_D 
				&& flags[3] == 0 )
			{
				fprintf(stderr, "Invalid arguments (-D)\n%s\n", invoke);
				exit( EXIT_FAILURE );
			} 
			else filedelay = atoi( argv[i+1] );
			flags[3] = 1;
		}
		else if( !strcmp( argv[i], "-C" ) )
		{
			if( flags[4] == 1 )
			{
				fprintf(stderr, "Invalid arguments (-C)\n%s\n", invoke);
				exit( EXIT_FAILURE );
			}
			flags[4] = 1;
			CACHE_FLAG = 1;
		}
		else
		{
			for( int j = 0; j < 4; ++j )
			{
				if( flags[j] == 0 )
				{
					fprintf(stderr, "Invalid arguments\n%s\n", invoke);
					exit( EXIT_FAILURE );
				}
			}
			break;
		}
	}
	/* done parsing args onto parsing file names */
	for( int i = 9; i < argc; ++i )
	{
		if( CACHE_FLAG == 1 ) ++i;
		Node *n = new Node( argv[i] );
		file_list->push_front( n );
		
	} // file_list is now populated with all files
	
	/* create threads */
	pthread_t thinker[3];
	pthread_create( &reader, NULL, &ReaderThread, NULL );
	for( int i = 0; i < 3; ++i ) pthread_create( &thinker[i], NULL, &ThinkerThread, NULL );
	pthread_join( reader, NULL );
	for( int i = 0; i < 3; ++i ) pthread_join( thinker[i], NULL );
	
	/* sort */
	list<IPNode*>::iterator iter;
	list<IPNode*>::iterator iter2;
	int occurence = 0;
	data->sort( compare_ip );
	fflush( stdout );
	data->sort( compare_ip );
	list<IPNode*> *temp = new list<IPNode*>();	
	for( iter = data->begin(); iter != data->end();  ++iter )
	{
		IPNode *ip = new IPNode( *iter );
		temp->push_front( ip );
	}
	temp->unique( unique_ip );
	temp->sort( compare_ip );
	for( iter = temp->begin(); iter != temp->end(); ++iter )
	{
		occurence = 0;
		for( iter2 = data->begin(); iter2 != data->end(); ++iter2 )
		{
			if( unique_ip( (*iter), (*iter2) ) == true  )
			{
				//fprintf( stdout, "comparing:: %s :: %s TO %s :: %s\n", (*iter)->data, (*iter)->data2, (*iter2)->data, (*iter2)->data2 );
				++occurence;
			}
		}
		RNode *r = new RNode( (*iter)->data, (*iter)->data2, occurence );
		output->push_front( r );
	}
	
	list<RNode*>::iterator o_iter;
	fprintf( stdout, "# occurences #       IP      #      FQDN      #\n" );
	for( o_iter = output->begin(); o_iter != output->end();  ++o_iter )
	{
		fprintf( stdout, "#    %d      #      %s       #      %s      #\n",
				  (*o_iter)->o, (*o_iter)->data, (*o_iter)->data2 );
	}
	/* print the cache */
	if( CACHE_FLAG == 1 )
	{
		fflush( stdout );
		fprintf( stdout, "#          Cache status          #\n" );
		fprintf( stdout, "#       IP      #      FQDN      #\n" );
		for( iter = cache->begin(); iter != cache->end();  ++iter )
		{
			fprintf( stdout, "#  %s  #  %s  #\n", (*iter)->data, (*iter)->data2 );
		
		}
	}
	
	/* done */
	exit( EXIT_SUCCESS );
}

/**
 * The driver for the Reader thread.
 * 
 * The logic of the thread is as follows:
 * First we open the file specified for reading only.
 * Once opened it reads out lines 1 at a time until none remain or 
 * 	b have been read.
 * Each line is tested using check_ip (see helper.cc for implementation)
 * Invalid lines are ignored, a valid line is stored to the buffer.
 * 
 * Once all b lines are read or none remain in the files then the thread
 * 	terminates.
 */
void *ReaderThread( void *v )
{
	char * line = NULL;
	size_t len = 0;
    ssize_t read;
	Mon->Acquire();
	while( file_list->empty() != true )
	{
		Mon->Release();
		/* open each file to read it */
		FILE *f;
		Mon->Acquire();
		f = fopen( file_list->front()->data, "r" );
		Mon->Release();
		if( f == NULL )
		{
			fprintf( stderr, "Failed to open the file for reading: %s\n", strerror(errno) );
			pthread_exit( NULL );
		}
		
		/* read out numlines lines from f */
		int count = 0;		
		while( ( read = getline( &line, &len, f ) ) )
		{
			char tmp[33];
			memset( tmp, 0, 33 );
			strncpy( tmp, line, strlen( line ) - 1 );
			
			/* validity check */
			if( check_ip( tmp ) != 0 || strlen( line ) > MAX_LINE_SIZE )
			{
				fprintf( stderr, "Invalid IP detected (%s)! Value will be ignored.\n", tmp);
				continue;
			}
			
			/* push the node to the buffer for thinkers */
			Node *n = new Node( tmp );
			Mon->Acquire();
			buffer->push_front( n );
			Mon->Release();
			if( (++count) > numlines - 1 ) break;
		}
		
		/* done with the first file pop it off and look for another */
		fclose(f);		
		Mon->Acquire();
		file_list->pop_front();
		Mon->Release();
		
		/* zzzZZZ */
		struct timespec time;
		time.tv_sec = filedelay/1000;
		time.tv_nsec = 0;
		nanosleep( &time, NULL );
	}
	pthread_exit( NULL );	
}

/**
 * The driver for Thinker threads
 *
 * The logic of the Thinker thread is as follows:
 * First we inspect the size of the buffer (where IPs are stored)
 * If it is empty we check if the reader has completed
 * If it has completed than we now know that all data has been
 * 	extracted from the buffer and can exit.
 * If either statement is false then there is more data and the
 * 	thread continues on and reads data from the buffer.
 * 
 * After which is compares the IP in the data it read to all
 *	values in the cache until it hits or has exhausted all
 *	values in the cache. On a hit it moves the hit node to the
 * 	front and pops the last entry from the list (which is LRU).
 * If the cache missed then we perform a DNS lookup using getaddrinfo()
 * 	and getnameinfo(). 
 * 
 * NOTE: this does not use the gethost.c code but rather an implementation 
 *	using the nondeprecated calls that was designed for use in CS4461.
 * Once the lookup is complete we add the new node to the lists
 * 	for the data and the cache. Again popping the tail off the
 * 	list to achieve LRU functionality.
 */
void *ThinkerThread( void *v )
{
	while( 1 )
	{
		int HIT = 0;
		Mon->Acquire();
		/* check if we need to do anything */
		/* if both these conditions are TRUE the reader is done and so are we */
		if( buffer->empty() == true )
		{
			Mon->Release();
			int *ret = new int( 0 );
			pthread_tryjoin_np( reader, (void**)&ret );
			if( *ret == 0 )
			{
				break;
			}
			else continue;
		}
		else
		{
			Mon->Release();
			Mon->Acquire();
			char ip_storage[33], fqdn_storage[512];
			strcpy( ip_storage, buffer->front()->data );
			buffer->pop_front();
			
			/* check in the cache for this IP */
			if( cache->empty() != true ) // stuff in the cache?
			{
				list<IPNode*>::iterator iter;
				int iter_c = 0;
				for( iter = cache->begin(); iter != cache->end();  ++iter ) // iterate over it
				{
					if( ( strcmp( (*iter)->data, ip_storage ) == 0 ) && HIT == 0 ) // cache hit
					{
						HIT = 1;						
						IPNode *t = new IPNode( (*iter)->data, (*iter)->data2 );
						cache->erase( iter );
						cache->push_front( t );
						data->push_front( cache->front() );
						if( (int)cache->size() >= cachesize ) cache->pop_back(); // drop the LRU node
					} 
					if ( ++iter_c >= (int)cache->size() || HIT == 1 ) break;
				} 
			}
			Mon->Release();
			
			/* do the DNS lookup */
			struct addrinfo hints;  // create struct
			memset(&hints, 0, sizeof(hints)); // fill struct with zeros
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_family = AF_UNSPEC;	
			struct addrinfo *results;
			int error;
			
			/* get info on the IP specified */
			error = getaddrinfo( ip_storage, NULL, &hints, &results );
			if(error != 0)
			{
				fprintf(stderr, "Error in getaddrinfo() -- %s: %s\n", ip_storage, gai_strerror(error)); 
				pthread_exit( NULL );
			}
			
			/* retrieve the FQDN */
			struct sockaddr_in *sa_in = (struct sockaddr_in *) results->ai_addr;
			if( getnameinfo( (struct sockaddr *) sa_in, INET_ADDRSTRLEN, fqdn_storage, sizeof( fqdn_storage ), NULL, 0, 0 ) )
			{
				fprintf( stderr, "Error in getnameinfo(): %s\n", gai_strerror(errno));
				pthread_exit( NULL );
			}
			
			/* store it to the data list */
			IPNode *ip = new IPNode( ip_storage, fqdn_storage );
			Mon->Acquire();
			data->push_front( ip );
			/* check if the cache is full and then push the new result to the cache */
 			/* HIT is used to flag that we already inserted this node to the list */
			if( (int)cache->size() < cachesize && HIT == 0 )
			{
				cache->push_front( ip );
			}
			else if ( HIT == 0 ) /* cache full pop off the back after insert */
			{
				cache->push_front( ip );
				cache->pop_back();
			}
			Mon->Release();
		}
		
		/* zzzZZZ */
		struct timespec time;
		time.tv_sec = threaddelay/1000;
		time.tv_nsec = 0;
		nanosleep( &time, NULL );
	}
	pthread_exit( NULL ); // should never get here but compiler is being annoying
}

/**
 * Helper function for comparing nodes.
 * 
 * Scans the two until it reaches a different character between the two.
 * When this occurs it returns whether first is lexicograpghically less
 * than second. Eg if first's FQDN was "a" and second's "b" it would return
 * true.
 */
bool compare_ip( const IPNode* first, const IPNode* second )
{
	int i = 0;
	while( first->data2[i] == second->data2[i] ) ++i;
	return first->data2[i] < second->data2[i];
}

bool unique_ip( const IPNode* first, const IPNode* second )
{
	if( !strcmp( first->data, second-> data ) && !strcmp( first->data2, second->data2 ) )
	{
		return true;
	} else return false;
}
// strcmp is a terrible function and i hate it