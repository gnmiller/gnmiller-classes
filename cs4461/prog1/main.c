// Greg Miller
// Program 1
// 1/24/15
// CS4461 - Computer Networks

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>

#define BIG 0
#define LITTLE 1

/**
 * Checks whether the given system is little or big endian.
 * 
 * NOTE: We assume the network uses big-endian
 * Returns 1 if the system is big endian and 0 if little endian.
 */
int checkEndian()
{
	uint32_t x = 1;
	if ( x == htonl(x) )
	{
		//printf("big-endian\n");
		return BIG;
	}
	else 
	{
		//printf("little-endian\n");
		return LITTLE;
	}
}

int main(int argc, char *argv[])
{
	/* check for correct invocation */
	if( argc != 2 )
	{
		fprintf( stderr, "Invalid arguments. Usage: dnsresolve <host>\n" );
		errno = EINVAL;
		exit(EXIT_FAILURE);
	}
	
	/* set up hints struct for call to getaddrinfo() and call it */
	struct addrinfo hints;  // create struct
	memset(&hints, 0, sizeof(hints)); // fill struct with zeros
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_UNSPEC;	
	struct addrinfo *results;
	int error;
	
	error = getaddrinfo( argv[1], NULL, &hints, &results );
	if(error != 0)
	{
		fprintf(stderr, "Error in getaddrinfo(): %s\n", gai_strerror(error)); 
		exit(EXIT_FAILURE);
	}
	
	/* return the endian-ness of the system */
	if( checkEndian() == BIG ) printf("The system and network are big-endian (MSB first).\n");
	else printf("The system is little-endian (LSB first), the network is big-endian (MSB first).\n");
	
	struct addrinfo *loop;
	struct sockaddr_in *sa_in;
	char ip_buf[512], rev_dns_buf[512];
	/* loop over all nodes in the linked list returned from getaddrinfo() */
	for( loop = results; loop != NULL; loop = loop->ai_next )
	{
        sa_in = (struct sockaddr_in *) loop->ai_addr;
		
		/* ipv4 */
		if( loop->ai_family == AF_INET )
		{		
			/* get the IP address from the encoded data */
			if ( inet_ntop( AF_INET, &(sa_in->sin_addr), ip_buf, INET_ADDRSTRLEN ) == NULL )
			{
				fprintf( stderr, "Error translating name (inet_ntop): %s\n", strerror(error));
				exit( EXIT_FAILURE );
			}			
			/* reverse DNS on the IP */
			if( getnameinfo( (struct sockaddr *) sa_in, INET_ADDRSTRLEN, rev_dns_buf, sizeof(rev_dns_buf), NULL, 0, 0 ) )
			{
				fprintf( stderr, "Error in getnameinfo(): %s\n", gai_strerror(errno));
				exit( EXIT_FAILURE );
			}
			fprintf( stdout, "ipv4: %s -- reverse dns lookup: %s\n", ip_buf, rev_dns_buf );
		}
		/* ipv6 */
		else 
		{
			/* get the IP address from the encoded data */
			if ( inet_ntop( AF_INET6, &(sa_in->sin_addr), ip_buf, INET6_ADDRSTRLEN ) == NULL )
			{
				fprintf( stderr, "Error translating name (inet_ntop): %s\n", strerror(error));
				exit( EXIT_FAILURE );
			}
			/* reverse DNS on the IP */
			if( getnameinfo( (struct sockaddr *) sa_in, INET6_ADDRSTRLEN, rev_dns_buf, sizeof(rev_dns_buf), NULL, 0, 0 ) )
			{
				fprintf( stderr, "Error in getnameinfo(): %s\n", gai_strerror(errno));
				exit( EXIT_FAILURE );
			}
			fprintf( stdout, "ipv6: %s -- reverse dns lookup: %s\n", ip_buf, rev_dns_buf );	
		}
		if( loop->ai_next == NULL ) break;
    }
	freeaddrinfo(results);
	exit( EXIT_SUCCESS );
}
