/** 
 * Greg Miller
 * gnmiller
 * 
 * CS4461
 * Spring 2015
 * Program 3
 * 
 * A simple http server.
 * Opens a socket, listens, and accepts on it
 * on accept creates a thread that then reads the http request,
 * and attempts to respond
 * 
 * will attempt to read and respond to html, jpg/jpeg requests others
 * are treated as text and responded with as best as possible
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <pthread.h>

/* some globaly parameters */
#define NEWLINE "\r\n"
#define MAX_CONN 50
#define DEFAULT_PORT 50001
#define DEFAULT_MSG "Hello world!\n"
#define TIMEOUT 120

/* global data; global so sighandler can free */
char *msg;   
pthread_t *threads;
time_t *startTime;

/* globals for threads */
int *arg_c, threadCount;
pthread_mutex_t thread_lock;

/* function prototypes */
void *threadFunc( void* parm );
int readLine( int fd, char **buffer, int *size, int *max );
void error(const char *msg){ perror(msg); exit(1); }
void sig_int_handler( int sig );

int main(int argc, char **argv){
    int sockfd, portno;
	msg = malloc( sizeof(char) * 512 );
	
	/* thread stuff */
	int threadMax = 20;
	threadCount = 0;
	threads = malloc( sizeof(pthread_t) * threadMax );
	startTime = malloc( sizeof(time_t) * threadMax );
	memset( threads, 0, sizeof( pthread_t ) * threadMax );
	memset( threads, 0, sizeof( time_t ) * threadMax );
	arg_c = &argc;
	pthread_mutex_init( &thread_lock, NULL );
	
	/* socket setup */
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	if( argc > 3 ) exit( EXIT_FAILURE );
	
	if( argc < 2 )
	{
		portno = DEFAULT_PORT;
		strcpy( msg, DEFAULT_MSG );
	}
	else
	{
		/* strtol() */
		errno = 0;
		char **endptr, *str;
		str = argv[1]; msg=argv[2]; endptr = NULL;
        	portno = strtol( str, endptr, 10 );

        	/* Check for various possible errors */
		if ((errno == ERANGE && (portno == LONG_MAX || portno == LONG_MIN)) || (errno != 0 && portno == 0)) 
		{
			perror("strtol");
			exit(EXIT_FAILURE);
		}

		if ((char*)endptr == str) 
		{
			fprintf( stderr, "No digits were found\n");
			exit(EXIT_FAILURE);
		}
	}
	
	/* open the socket */
    	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    	if (sockfd < 0){
        	error("error opening socket");
		exit(1);
	}
	
	/* setup the port */	
    	memset( &serv_addr, 0, sizeof(serv_addr));	 
	 
	/* setup info for server */
    	serv_addr.sin_family = AF_INET;
    	serv_addr.sin_addr.s_addr = INADDR_ANY;
    	serv_addr.sin_port = htons(portno);
    	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{ 
		perror( "Error binding socket:");
		exit(1);
	}
	
	/* set some socket options */
	int optLevel;
	optLevel = 1;
	setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR, &optLevel, sizeof( optLevel ) );
	
	/* start listening */
	listen(sockfd,5);
	printf("SERVER: Listening on port %d\n", portno);
    	clilen = sizeof(cli_addr);

	int clifd;
	clifd = -1;
	signal( SIGINT, sig_int_handler );
	for( ;; )
	{
		/* wait for clients */
		fprintf( stdout, "SERVER: Waiting for a connection.\n");
		
		clifd = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen );
		if( sockfd < 0 )
		{
			perror("socket error");
			exit(1);		
		}
		
		/* client detected spawn a thread */
		fprintf( stdout, "SERVER: A client opened a new connection.\n");
		fprintf( stdout, "SERVER: Creating a new thread for this client.\n" );
		
		/* check if we can add it to the array */
		pthread_mutex_lock( &thread_lock );
		if( threadCount+1 > threadMax )
		{
			threadMax*=2;
			threads = realloc( threads, threadMax * sizeof( pthread_t ) );
			startTime = realloc( startTime, threadMax * sizeof( time_t) );
		}
		pthread_mutex_unlock( &thread_lock );
		
		/* create it */
		pthread_mutex_lock( &thread_lock );
		pthread_create( &threads[threadCount], 0, &threadFunc, (void*)clifd );
		startTime[threadCount] = time( NULL );
		++threadCount;
		pthread_mutex_unlock( &thread_lock );
		for( int i = 0; i < threadCount; ++i )
		{
			if( startTime[i] + (time_t)TIMEOUT < time( NULL ) )
			{
				fprintf( stdout, "SERVER: Thread has timed out. It's request and connection will be terminated.\n" );
				pthread_cancel( threads[i] );
			}
		}
		if( threadCount >= MAX_CONN )
		{
			fprintf( stdout, "SERVER: ERR too many connections. Terminating ALL connections.\n" );
			for( int i = 0; i < threadCount; ++i )
			{
				if( pthread_tryjoin_np( threads[i], NULL ) != 0 ) /* if this errors then the thread hasnt terminated */
				{
					pthread_cancel( threads[i] );
				}
				else
				{
					pthread_join( threads[i], NULL );
				}
				pthread_mutex_lock( &thread_lock );
				threads[i] = NULL;
				startTime[i] = (time_t)0;
				threadCount = 0;
				pthread_mutex_unlock( &thread_lock );
			}
			pthread_mutex_lock( &thread_lock );
			free( threads );
			free( startTime );
			threadMax = 20;
			threads = malloc( sizeof( pthread_t ) * threadMax );
			startTime = malloc( sizeof( time_t ) * threadMax );
			memset( threads, 0, sizeof( pthread_t ) * threadMax );
			memset( threads, 0, sizeof( time_t ) * threadMax );
			pthread_mutex_unlock( &thread_lock );
		}
	}
	fprintf( stdout, "SERVER: Connection closed.\n" );
	
	free( msg );
	return EXIT_FAILURE; // we should be inf looping and waiting for ctrl to term
}

/* read data from a file descriptor until a new line 
 *
 * fd - fd to read
 * buffer - where to store the data
 * size - how much the buffer currently holds
 * max - how much the buffer can hold before needing to realloc
 *
 * NOTE: does not use recv() but rather read() which in this
 * 	case would be equivalent as read( fd, buf, size ) is equivalent
 * 	to recv( fd, buf, size, 0 )
 */
int readLine( int fd, char **buffer, int *size, int *max )
{
	int recv_size, ret;
	recv_size = ret = 0;
	char tmp[32];
	
	/* clean out the buffer from any previous calls and set size correctly */
	*size = 0;
	memset( *buffer, 0, *max );
	memset( tmp, 0, 32 );
	
	/* read until a new line */
	recv_size = read( fd, tmp, 1 );
	strncat( *buffer, tmp, 1 );
	if( recv_size == 0 ) return -1;
	while( strstr( *buffer, NEWLINE ) == NULL ) /* search for the new line */
	{
		/* realloc the buffer if needed */
		++ret;
		if( ++*size > *max )
		{
			*max = *max * 2;
			*buffer = realloc( *buffer, *max );
		}
		
		
		/* get the next char */
		recv_size = read( fd, tmp, 1 );
		strncat( *buffer, tmp, 1 );
		if( recv_size < 0 )
		{
			return -1;
		}
	}
	if( strcmp( "\r\n", *buffer ) == 0 ) return -2;
	return ret;
}

/**
 * signal handler for ctrlc
 * when ctrl is caught the application waits for all processes 
 * still running (pthread_join()). It then attempts to close every
 * open file descriptor that it can. While this is not particularly
 * efficient it's the most straightforward since we can safely ignore
 * any failed calls to close() with EBADF, and we're terminating anyway
 * other errors are irrelevant to us now.
 */
void sig_int_handler( int sig )
{
	signal( sig, SIG_IGN );
	fprintf( stdout, "\nCtrl-C caught cleaning up.\n" );
	
	/* kill threads still running */
	fprintf( stdout, "SERVER: Waiting on all threads.\n" );
	for( int i = 0; i < threadCount; ++i )
	{
		pthread_cancel( threads[i] );
	}
	/* close dangling fds from threads  */
	/* start at 3 to keep stdio open */
	fprintf( stdout, "SERVER: Closing all FDs in use.\n" );
	for( int i = 3; i < sysconf( _SC_OPEN_MAX ); ++i )
	{
		errno = 0;
		close( i );
		if( !errno ) /* errno is false means we killed some fd */
		{
			fprintf( stdout, "SERVER: fd closed (%d).\n", i );
		}
	}
	
	/* free .. */
	free( threads );
	free( startTime );
	free( msg );
	exit( EXIT_SUCCESS );
}

/*
 * driver function for threads
 * an integer is passed in through parm
 * the integer is stored 
 */
void *threadFunc( void *parm )
{
	/* intialization */
	int writeMax = 512, readMax = 512, readSize = 0, writeSize = 0;
	char *readBuffer = malloc( sizeof( char ) * readMax );
	char *writeBuffer = malloc( sizeof( char ) * writeMax );
	memset( writeBuffer, 0, writeMax );
	memset( readBuffer, 0, readMax );
	int retVal = 0;
	
	int my_fd = (int)parm;
	
	/* read the request (line by line) */
	char request[1024];
	memset( request, 0, 1024 );
	while( ( retVal = readLine( my_fd, &readBuffer, &readSize, &readMax ) ) > 0 )
	{
		if( retVal == -1 || retVal < -2 )
		{
			fprintf( stderr, "Error reading clients request." );
			pthread_exit( &retVal );
		}
		strcat( request, readBuffer );
	}
	
	/* response to request */
	#define GET "GET"
	#define HTTP "HTTP"
	fprintf( stdout, "THREAD: Received a request from a client:\n%s", request );
	if( strstr( request, GET ) && strstr( request, HTTP ) )
	{			
		/* automatically send the proper message for this case */
		if( *arg_c == 3 )
		{
			/* response */
			sprintf( writeBuffer, "HTTP/1.1 200 OK%sContent-type: text/plain%sConnection: close%sContent-length: %d%s%s%s%s",
					NEWLINE, NEWLINE, NEWLINE, (int)strlen(msg), NEWLINE, NEWLINE, msg, NEWLINE );
			fprintf( stdout, "THREAD: Response:\n%s", writeBuffer );
			write( my_fd, writeBuffer, strlen( writeBuffer ) );
		}
			
		/* check the response */
		else if( strncmp( request, "GET", 3 ) == 0 ) /* try to get the file and return it */
		{
			if( strlen( request ) > 1024 )
			{
				fprintf( stderr, "Path too long, I give up!\n" );
				goto err404;
			}
				
			/* build the local path to the requested file */
			char *path;
			path = malloc( sizeof( char ) * 512 );
			memset( path, 0, 512 );
			strcat( path, "." );
			strtok( request, " " );
			char *tmp = strtok( NULL, " " );
			strcat( path, tmp );
			char *tmp_str;
			tmp_str = path;
			if( path[strlen(path)-1] == '/' ) sprintf( path, "%sindex.html", tmp_str );
			if( strstr( "..", path ) != NULL )
			{
				fprintf( stderr, "Invalid path! Contains \"..\"\n" );
				free( path );
				goto err404;
			}
			/* psath is acceptable try to open for reading */
			else
			{
				int openfd;
				off_t f_size;
				openfd = open( path, O_RDONLY );
				memset( writeBuffer, 0, writeMax );	
				
				/* open the path specified in the request for reading */
				if( openfd < 0 )
				{
					fprintf( stderr, "Error couldn't open path for reading.\n" );
					free( path );
					goto err404;
				}
				struct stat f_info;
			
				/* get the size (not ideal) */
				fstat( openfd, &f_info );
				f_size = f_info.st_size;
				char *t_buf;
				t_buf = malloc( sizeof( char ) * f_size );
				memset( t_buf, 0, f_size );
					
				/* read into mem */
				read( openfd, t_buf, f_size );
					
				/* structure the response header */
				memset( writeBuffer, 0, writeMax );
				if( writeMax + 64 < (int)f_size )
				{	
					writeMax*=3;
					writeBuffer = realloc( writeBuffer, writeMax+10 );
					if( writeBuffer == NULL )
					{
						free( writeBuffer );
						pthread_exit( &retVal );
					}
				}
				/* check if jpg */
				if( strstr( path, ".jpg" ) || strstr( path, ".jpeg" ) )
				{
					sprintf( writeBuffer, "HTTP/1.1 200 OK%sContent-type: image/jpeg%sConnection: close%sContent-length: %d%s%s",
						NEWLINE, NEWLINE, NEWLINE, (int)f_size, NEWLINE, NEWLINE );
				}
				/* check if html */
				else if( strstr( path, ".html" ) )
				{
					sprintf( writeBuffer, "HTTP/1.1 200 OK%sContent-type: text/html%sConnection: close%sContent-length: %d%s%s",
						NEWLINE, NEWLINE, NEWLINE, (int)f_size, NEWLINE, NEWLINE );
				}
				/* check if ico (favicon) */
				else if( strstr( path, ".ico" ) )
				{
					sprintf( writeBuffer, "HTTP/1.1 200 OK%sContent-type: image/vnd.microsoft.icon%sConnection: close%sContent-length: %d%s%s",
						NEWLINE, NEWLINE, NEWLINE, (int)f_size, NEWLINE, NEWLINE );
				}
				/* anything else is plaintext */
				else
				{
					sprintf( writeBuffer, "HTTP/1.1 200 OK%sContent-type: text/plain%sConnection: close%sContent-length: %d%s%s",
						NEWLINE, NEWLINE, NEWLINE, (int)f_size, NEWLINE, NEWLINE );
				}
				
				/* send the response out */
				write( my_fd, writeBuffer, strlen(writeBuffer) );
				fprintf( stdout, "THREAD: Response Header -- \n%s", writeBuffer );
				write( my_fd, t_buf, f_size );
				write( my_fd, "\r\n\r\n", 4 );
				free( path );
				free( t_buf );
				free( writeBuffer );
				free( readBuffer );
				fprintf( stdout, "THREAD: Connection closed.\n" );
				close( my_fd );
				retVal = 0;
				pthread_mutex_lock( &thread_lock );
				--threadCount;
				pthread_mutex_unlock( &thread_lock );
				pthread_exit( &retVal );
			}
		}
		/* invalid response */
		else
		{
			err404: ;					
			char *msg_404 = "Sorry! You request was not valid!\r\n";
			sprintf( writeBuffer, "HTTP/1.1 404 Not found%sContent-type: text/plain%sonnection: close%sContent-length: %d%s%s%s%s",
					NEWLINE, NEWLINE, NEWLINE, (int)strlen( msg_404 ), NEWLINE, NEWLINE, msg_404, NEWLINE );
			write( my_fd, writeBuffer, strlen( writeBuffer ) );
			fprintf( stderr, "Did not receive a valid request.\nThe client has also been notified and the connection closed.\n" );
			close( my_fd );
			free( writeBuffer );
			free( readBuffer );
			retVal = -1;
			pthread_mutex_lock( &thread_lock );
			--threadCount;
			pthread_mutex_unlock( &thread_lock );
			pthread_exit( &retVal );
		}
		fprintf( stdout, "THREAD: Completed processing request. Thread terminating\n" );
		retVal = 0;
		pthread_mutex_lock( &thread_lock );
		--threadCount;
		pthread_mutex_unlock( &thread_lock );
		pthread_exit( &retVal );
	}
}
