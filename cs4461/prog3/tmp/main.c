/** 
 * Greg Miller
 * gnmiller
 * 
 * CS4461
 * Spring 2015
 * Program 3
 * 
 * A simple http server.
 * 
 * Loosely based on code written for a web based card guessing game
 * written for CS3411.
 *
 * The only additional feature is that when invoked with no options the
 * the application runs with a default port number defined (50001) and runs
 * as normal if only a port was specified.
 *
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

/* these are global so we can use a sig handler of ctrlc to ensure no mem leaks */
char *readBuffer, *writeBuffer, *msg;
int *clifd_ptr;
#define NEWLINE "\r\n"
int readLine( int fd, char **buffer, int *size, int *max );

void error(const char *msg){ perror(msg); exit(1); }
void sig_int_handler( int sig );

int main(int argc, char **argv){
    int sockfd, portno;
	msg = malloc( sizeof(char) * 512 );
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
	if( argc > 3 ) exit( EXIT_FAILURE );
	/* default */
	#define DEFAULT_PORT 50001
	#define DEFAULT_MSG "Hello world!\n"
	if( argc < 2 )
	{
		portno = DEFAULT_PORT;
		strcpy( msg, DEFAULT_MSG );
	}
	else
	{
		errno = 0;
		char **endptr;
		char *str;
		str = argv[1];
		msg=argv[2];
		endptr = NULL;
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
	
	char *readBuffer, *writeBuffer;
	int readSize, readMax, writeMax, clifd; 
	readSize = 0;
	clifd = -1;
	clifd_ptr = &clifd;
	readMax = writeMax = 512;
	readBuffer = malloc( sizeof( char ) * readMax );
	writeBuffer = malloc( sizeof( char ) * writeMax );
	signal( SIGINT, sig_int_handler );
	for( ;; )
	{
		memset( readBuffer, 0, readMax );
		memset( writeBuffer, 0, writeMax );
		
		/* wait for clients */
		fprintf( stdout, "SERVER: Waiting for a connection.\n");
		clifd = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen );
		if( sockfd < 0 )
		{
			perror("socket error");
			exit(1);		
		}
		fprintf( stdout, "SERVER: A client opened a new connection.\n");
		
		/* get the reuqest */
		int retVal = 0;
		char request[1024];
		memset( request, 0, 1024 );
		while( ( retVal = readLine( clifd, &readBuffer, &readSize, &readMax ) ) > 0 )
		{
			if( retVal == -1 || retVal < -2 )
			{
				fprintf( stderr, "Error reading clients request." );
				exit( EXIT_FAILURE );
			}
			strcat( request, readBuffer );
		}
		
		/* response to request */
		#define GET "GET"
		#define HTTP "HTTP"
		fprintf( stdout, "SERVER: Received a request from a client:\n%s", request );
		if( strstr( request, GET ) && strstr( request, HTTP ) )
		{			
			/* automatically send the proper message for this case */
			if( argc == 3 )
			{
				/* response */
				sprintf( writeBuffer, "HTTP/1.1 200 OK%sContent-type: text/plain%sConnection: close%sContent-length: %d%s%s%s%s",
						NEWLINE, NEWLINE, NEWLINE, (int)strlen(msg), NEWLINE, NEWLINE, msg, NEWLINE );
				fprintf( stdout, "SERVER: Response:\n%s", writeBuffer );
				write( clifd, writeBuffer, strlen( writeBuffer ) );
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
				/* path is acceptable try to open for reading */
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
							exit( EXIT_FAILURE );
						}
					}
					if( strstr( path, ".jpg" ) || strstr( path, ".jpeg" ) )
					{
						sprintf( writeBuffer, "HTTP/1.1 200 OK%sContent-type: image/jpeg%sConnection: close%sContent-length: %d%s%s",
							NEWLINE, NEWLINE, NEWLINE, (int)f_size, NEWLINE, NEWLINE );
					}
					else if( strstr( path, ".html" ) )
					{
						sprintf( writeBuffer, "HTTP/1.1 200 OK%sContent-type: text/html%sConnection: close%sContent-length: %d%s%s",
							NEWLINE, NEWLINE, NEWLINE, (int)f_size, NEWLINE, NEWLINE );
					}
					else
					{
						sprintf( writeBuffer, "HTTP/1.1 200 OK%sContent-type: text/plain%sConnection: close%sContent-length: %d%s%s",
							NEWLINE, NEWLINE, NEWLINE, (int)f_size, NEWLINE, NEWLINE );
					}
					/* send the response out */
					write( clifd, writeBuffer, strlen(writeBuffer) );
					fprintf( stdout, "SERVER: Response Header -- \n%s", writeBuffer );
					write( clifd, t_buf, f_size );
					write( clifd, "\r\n\r\n", 4 );
					free( path );
					free( t_buf );
					fprintf( stdout, "SERVER: Connection closed.\n" );
					close( clifd );
				}
			}
			/* invalid response */
			else
			{
				err404: ;					
				char *msg_404 = "Sorry! You request was not valid!\r\n";
				sprintf( writeBuffer, "HTTP/1.1 404 Not found%sContent-type: text/plain%sonnection: close%sContent-length: %d%s%s%s%s",
							NEWLINE, NEWLINE, NEWLINE, (int)strlen( msg_404 ), NEWLINE, NEWLINE, msg_404, NEWLINE );
				write( clifd, writeBuffer, strlen( writeBuffer ) );
				fprintf( stderr, "Did not receive a valid request.\nThe client has also been notified and the connection closed.\n" );
				close( clifd );
			}
		}
	}
	close( clifd );
	fprintf( stdout, "SERVER: Connection closed.\n" );
	free( readBuffer );
	free( writeBuffer );
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

int fd_is_valid(int fd)
{
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
}

void sig_int_handler( int sig )
{
	signal( sig, SIG_IGN );
	fprintf( stdout, "\nCtrl-C caught cleaning up.\n" );
	if( fd_is_valid( *clifd_ptr ) )
	{
		close( *clifd_ptr );
		fprintf( stdout, "SERVER: Connection closed\n");
	}
	free( readBuffer );
	free( writeBuffer );
	free( msg );
	exit( EXIT_SUCCESS );
}
