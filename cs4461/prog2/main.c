/**
 * Greg  Miller -- gnmiller
 * Cs4461 -- Computer Networks
 * Spring 15
 * 2/11/15
 * Program 2
 * 
 * FILE: main.c
 * 
 * executed as:
 * 	httpget [host_name] [port_num]
 * 	if no host or port is specified a default is supplied,
 * 	the defaults are specified in the macros DEFAULT_PORT
 * 	and DEFAULT_HOST
 * 	
 * The application retrieves the IP address of host_name,
 * connects a socket to that host on port_num. Once connected
 * a request (statically coded) is sent for the page "/"
 * via HTTP 1.1.
 * 
 * The header is parsed for information, specifically the presence
 * of chunked encoding. Based on the results of this the application
 * then either reads the socket line by line until read() returns 0
 * (the socket has been closed by the host).
 * 
 * If chunked encoding is detected the application attempts to read
 * the chunk size, and then 1 character at a time reads chunk_size
 * characters from the socket until the socket closes or a chunk size of
 * 0 is read.
 * 
 * In both cases the line (or chunk) is printed to the stdout after it has
 * been read.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>

#define PRINT_PREFIX "##### "
#define NEW_LINE "\r\n"
#define HEX 16
#define DEC 10

/* read until \n from a fd */
int readLine( int fd, char **buffer, int* size, int *max );

int main( int argc, char** argv )
{
	int port_num, ret, CHUNKED;
	char host_name[256];
	/* check the arguments and correct errors if possible
	 * in the event that invalid arguments are detected (or some
	 * are left unspecified) the application makes assumptions and
	 * tries to accomodate.
	 * 
	 * these assumptions being that if no server is specified google.com
	 * is used, if no port is specified 80 is used. These defaults can be
	 * modified through the corresponding macros below
	 */
	#define DEFAULT_PORT 80
	#define DEFAULT_HOST "google.com"
	if( argc == 3 )
	{
		char *end_ptr;
		port_num = strtol( argv[2], &end_ptr, DEC );
		/* these checks were taken directly from the strtol man page */
		if ((errno == ERANGE && (port_num == LONG_MAX || port_num == LONG_MIN))
                   || (errno != 0 && port_num == 0))
		{
			fprintf( stderr, "%sDetected over or underflow in port number. Using default of %d\n",
					 PRINT_PREFIX, DEFAULT_PORT );
			port_num = DEFAULT_PORT;
		}
		if (end_ptr == argv[2]) {
            fprintf( stderr, "%sNo digits were found. Using default port of %d\n",
					 PRINT_PREFIX, DEFAULT_PORT );
			port_num = DEFAULT_PORT;
        }
		strcpy( host_name, argv[1] );
	}
	else if( argc == 1 )
	{
		strcpy( host_name, DEFAULT_HOST );
		port_num = DEFAULT_PORT;
	}
	else if( argc != 3 )
	{
		fprintf( stderr, "Invalid arguments\n" );
		exit( EXIT_FAILURE );
	}
	
	/* set up hints struct for call to getaddrinfo() and call it */
	struct addrinfo hints;  // create struct
	memset(&hints, 0, sizeof(hints)); // fill struct with zeros
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC;	
	struct addrinfo *results;
	int error;
	
	error = getaddrinfo( host_name, NULL, &hints, &results );
	if(error != 0)
	{
		fprintf(stderr, "Error in getaddrinfo(): %s\n", gai_strerror(error)); 
		exit(EXIT_FAILURE);
	}
	
	/* put the IP somewhere for later */
	struct in_addr addr;
	addr.s_addr = ((struct sockaddr_in *)(results->ai_addr))->sin_addr.s_addr;
	char fetched_addr[256];
	strcpy( fetched_addr, inet_ntoa( addr ) );
	
	/* create a TCP socket to read from */
	int sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if( sockfd < 0 )
	{
		fprintf( stderr, "Could not create socket: %s\n", strerror(errno) );
	}
	
	/* preparing to connect to the server */
	struct sockaddr_in host;
	host.sin_family = AF_INET;
	host.sin_port = htons( port_num ); // load the port
	inet_aton( fetched_addr, &host.sin_addr ); // load the IP
	
	/* actually connect to the server */
	error = 0;
	error = connect( sockfd, &host, sizeof( struct sockaddr ) );
	if( error < 0 )
	{
		fprintf( stderr, "Error connecting to host: %s\n", strerror(errno) );
		exit( EXIT_FAILURE );
	}
	
	char request[256];
	char *recv_data;
	int recv_max, recv_size;
	recv_max = 512;
	recv_size = 0;
	recv_data = (char*) malloc( recv_max * sizeof( char ) );
	memset( recv_data, 0, recv_max );
	
	sprintf( request, "GET / HTTP/1.1%sHost: %s:%d%sConnection: close%s%s", NEW_LINE, host_name, port_num, NEW_LINE, NEW_LINE, NEW_LINE );
	
	/* request the page */
	#ifdef DEBUG
	fprintf( stdout, "%sTransmitting request to the server: %s", PRINT_PREFIX, request );
	#endif
	if( send( sockfd, request, strlen( request ), 0 ) < 0 )
	{
		fprintf( stderr, "Error sending on the socket (request): %s", strerror(errno) );
		exit( EXIT_FAILURE );
	}
	
	/* start reading the socket */
	ret = CHUNKED = 0;
	while( ( ret = readLine( sockfd, &recv_data, &recv_size, &recv_max ) ) != -2 )
	{
		if( ret == -1 )
		{
			fprintf( stderr, "Did not read any data from the socket, was it closed on us?\n" );
			exit( EXIT_FAILURE );
		}
		if( ret == -3 )
		{
			fprintf( stdout, "%sBlank line detected in response from host.\n", PRINT_PREFIX );
			break;
		}
		printf( "%s\n", recv_data );	
		
		/* check if chunked encoding (in header ONLY ) */
		/* only check against NULL because it doesnt really matter where it is just that its there */
		if ( strcasestr( recv_data, "Transfer-Encoding: chunked" ) != NULL )
			CHUNKED = 1;
	} printf( "\n" );
	
	if( CHUNKED )
	{
		#ifdef DEBUG
		fprintf( stdout, "%sDetected chunked encoding attempting to read chunks.\n%sNext line is expected to be a chunk size.\n", PRINT_PREFIX, PRINT_PREFIX );
		#endif
		ret = readLine( sockfd, &recv_data, &recv_size, &recv_max );
		int chunk_size;
		chunk_size = 0;
		char *end_ptr;
		chunk_size = strtol( recv_data, &end_ptr, HEX );
		/* again these were directly taken from the man page */
		if ((errno == ERANGE && (chunk_size == LONG_MAX || chunk_size == LONG_MIN))
                   || (errno != 0 && chunk_size == 0))
		{
			fprintf( stderr, "%sDetected over or underflow in chunk size.\n", PRINT_PREFIX );
			exit( EXIT_FAILURE );
		}
		if (end_ptr == recv_data) {
            fprintf( stderr, "%sNo digits were found.\n", PRINT_PREFIX );
			exit( EXIT_FAILURE );
        }
        while( chunk_size > 0 )
		{
			/* read the chunk */
			recv_size = 0;
			if( recv_max < chunk_size+1 )
			{
				recv_data = realloc( recv_data, chunk_size+1 );
				recv_max = chunk_size+1;
			}
			
			memset( recv_data, 0, recv_max );
			char tmp[32];
			int read_count;
			read_count = 0;
			while( ( ret = recv( sockfd, tmp, 1, 0 ) ) > 0 && read_count < chunk_size ){
				++read_count;
				strncat( recv_data, tmp, 1 );
			}
			if( ret == 0 )
			{
				fprintf( stderr, "%sSocket was closed in an orderly fashion.\n", PRINT_PREFIX );
			}
			
			/* print the chunk */
			fprintf( stdout, "%s\n", recv_data );
			
			/* get the next chunk size */
			ret = readLine( sockfd, &recv_data, &recv_size, &recv_max );
			
			if( !strcasestr( recv_data, "\n" ) )
			{
				#ifdef DEBUG
				fprintf( stdout, "%sDetected newline in response.\n%sNext line should be a chunk size\n",
						 PRINT_PREFIX, PRINT_PREFIX );
				#endif
			}
			else
			{
				fprintf( stderr, "%sNext line was not blank!\n%sChunk size was most likely not correct!\n",
						PRINT_PREFIX, PRINT_PREFIX );
				exit( EXIT_FAILURE );
			}
			ret = readLine( sockfd, &recv_data, &recv_size, &recv_max );
			chunk_size = strtol( recv_data, &end_ptr, HEX );
			if ((errno == ERANGE && (chunk_size == LONG_MAX || chunk_size == LONG_MIN)))
			{
				fprintf( stderr, "%sDetected over or underflow in chunk size.\n", PRINT_PREFIX );
				exit( EXIT_FAILURE );
			}
			if (end_ptr == recv_data) {
				fprintf( stderr, "%sNo digits were found.\n", PRINT_PREFIX );
				exit( EXIT_FAILURE );
			}
		}
		#ifdef DEBUG
		fprintf( stdout, "%sDetected end of chunked response (chunk size == 0).\n", PRINT_PREFIX );
		fprintf( stdout, "%sReading until end of stream to check for footer.\n", PRINT_PREFIX );
		#endif
		while( ( ret = readLine( sockfd, &recv_data, &recv_size, &recv_max ) ) != -2 )
		{
			if( ret == -1 )
			{
				#ifdef DEBUG
				fprintf( stderr, "%sSocket appears to have closed.\n", PRINT_PREFIX );
				fprintf( stderr, "%sIf no other error messages appear we most likely reached the end of the response.\n", PRINT_PREFIX );
				#endif
				break;
			}
			if( strcmp( recv_data, NEW_LINE ) ) fprintf( stdout, "%s\n", recv_data );
		}
	}
	else
	{
		#ifdef DEBUG
		fprintf( stdout, "%sChunked encoding not detected, proceeding to read page line by line.\n", PRINT_PREFIX );
		#endif
		
		memset( recv_data, 0, recv_size );
		recv_size = 0;
		while( ( ret = readLine( sockfd, &recv_data, &recv_size, &recv_max ) ) != -2 )
		{
			if( ret == -1 )
			{
				#ifdef DEBUG
				fprintf( stderr, "%sSocket appears to have closed.\n", PRINT_PREFIX );
				fprintf( stderr, "%sIf no other error messages appear we most likely reached the end of the response.\n", PRINT_PREFIX );
				#endif
				break;
			}
			if( strcmp( recv_data, NEW_LINE ) ) fprintf( stdout, "%s\n", recv_data );
		}
		if( errno != 0 )
		{
			//fprintf( stderr, "Error when reading from the socket: %s\n", strerror(errno) );
			exit( EXIT_SUCCESS );
		}
		#ifdef DEBUG
		fprintf( stdout, "%sDetected end of non chunked response, cleaning up and terminating.\n",
				PRINT_PREFIX );
		#endif
	}
	free( recv_data );
	freeaddrinfo( results );
	exit( EXIT_SUCCESS );
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
 * 
 * returns -1 if the fd read 0 bytes
 * returns -2 for other failure
 * returns -3 if the line read was blank (eg \r\n)
 * otherwise returns the total size read
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
	if( recv_size == 0 ) return -1;	
	while( tmp[0] != '\n' && recv_size > 0 ) // we don't care about carriage returns? \r\n will still be \n on the end of the line
	{
		++ret;
		if( ++*size > *max ) // check if we can fit in the buffer
		{
			*max = *max * 2; // increase max size
			*buffer = realloc( *buffer, *max ); // realloc
		}
		strncat( *buffer, tmp, 1 );
		
		/* get the next char */
		recv_size = read( fd, tmp, 1 );
		if( recv_size < 0 )
		{
			return -2;
		}
	}
	
	if( ++*size > *max ) // check if we can fit in the buffer
	{
		*max = *max * 2; // increase max size
		*buffer = realloc( *buffer, *max ); // realloc
	}
	
	/* make sure that the output is actually formatted with a newline */
	if( ret == 1 )
	{
		if( tmp[0] == '\n' )
		{
			strcat( *buffer, "\n" );
			return -3;
		}
	}
	return ret;
}
