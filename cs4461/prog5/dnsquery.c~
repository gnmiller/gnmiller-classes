/**
 * Greg Miller
 * gnmiller
 * HW8 - DNS Resolution
 * 
 * Query a DNS server for results after constructing the packet.
 * 
 * Only implements part A of the assignment.
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include "dns.h"
#include "hexdump.h"

#define DEFAULT_DNS "8.8.8.8"
#define T_A 1 // we want an ipv4 host to be resolved

/* holds responses from the DNS server */
static res_t ans[20];

/* DNS server to query */
char dns[32];

int main( int argc, char **argv )
{
	//if( argc < 2 ) return 1;
    char str[64], target[64];
	
	int sockfd, len, qname_len;
	unsigned char *qname = NULL;
    unsigned char *buf = NULL;
    question_t *qinfo = NULL;

	sockfd = len = -1;
	qname_len = 0;
	
    struct sockaddr_in dest;
	
	if( argc == 3 )
	{
		strcpy( dns, argv[2] );
		strcpy( target, argv[1] );
	}
	if( argc == 2 )
	{
		strcpy( dns, DEFAULT_DNS );
		strcpy( target, argv[1] );
	}
	else
	{
		printf( "Usage: dnsquery target.address [dns.server]\n8.8.8.8 is used if no server is specified\n" );
		exit( 1 );
	}
	
	
    printf( "Resolving %s using %s\n", target, dns );
    buf = malloc( 1024 );

    sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ); // UDP socket
    fill_sockaddr( &dest ); // fill in the dns server info
    fill_dnshdr( (dns_header_t *) buf ); // fill in the dns header

    qname = (unsigned char*) (buf + sizeof( dns_header_t ) ); // point to after the header
    set_domain( qname, (unsigned char *)target ); // setup the domain name for the server to read
    qname_len = ( strlen( (const char*) qname) + 1 ); // get the length
    qinfo = (question_t*) ( buf + sizeof( dns_header_t ) + qname_len ); 
			//point to after the hdr and query
	
	// TODO: change this for part B?
    qinfo->qtype = htons( T_A ); // requesting a host resolution
    qinfo->qclass = htons( 1 ); // internet

	
	/* send our request to the server now that its been built */
    printf("Transmitting packet to server...\n");
    if ( sendto( sockfd, (char*) buf,
		sizeof(dns_header_t) + qname_len + sizeof(question_t),
		0, (struct sockaddr*) &dest, sizeof(dest)) < 0 )
    {
        perror("sendto failed");
    }
	hexdump( "Dumping contents of packet transmitted :: ", (char*) buf, 
		sizeof(dns_header_t) + qname_len + sizeof(question_t) );

    /* get the servers answer */
    len = sizeof( dest );
    printf("Receiving response from server...\n");
    if ( recvfrom( sockfd, (char*) buf, 1024, 0, (struct sockaddr*) &dest, (socklen_t*) &(len) ) < 0 )
    {
        perror("recvfrom failed");
    }
	hexdump( "Dumping contents of packet received :: ", (char*) buf, 1024 );
	
    parse_ans( (char *) buf, qname_len );
	//print_ans( ntohs( ( (dns_header_t *) buf )->ans_count ), str);
    free(buf);
    return 0;
}

/* fill the sockaddr_in struct for connecting to the socket
	port 53, and google dns are specified "8.8.8.8"
	in a more general application we might read the DNS servers
	off disk and use one or more of them */
void fill_sockaddr(struct sockaddr_in *tar)
{
    tar->sin_family = AF_INET;
    tar->sin_port = htons(53);
    tar->sin_addr.s_addr = inet_addr( dns );          //dns servers
}

/* fill the dnshdr struct with appropriate data
	see dns.h for struct specification */
void fill_dnshdr( dns_header_t *dnshdr )
{
    dnshdr->id = (unsigned short) htons(getpid());
    dnshdr->qr = 0;          //This is a query
    dnshdr->opcode = 0;          //This is a standard query
    dnshdr->aa = 0;          //Not Authoritative
    dnshdr->tc = 0;          //This message is not truncated
    dnshdr->rd = 1;          //Recursion Desired
    dnshdr->ra = 0;          //Recursion not available! hey we dont have it (lol)
    dnshdr->z = 0;
    dnshdr->ad = 0;
    dnshdr->cd = 0;
    dnshdr->rcode = 0;
    dnshdr->q_count = htons(1);          //we have only 1 question
    dnshdr->ans_count = 0;
    dnshdr->auth_count = 0;
    dnshdr->add_count = 0;
}

/* parse an address response from the DNS server
	since the server doesnt respond as www.example.com
	and uses 3www7example3com we translate to a human
	readable format with this */
char* read_name( unsigned char *read, unsigned char *buf, int *count )
{
    unsigned char *name;
    unsigned int p, jump, offset;
    int i, j;
	p = jump = offset = 0;

    *count = 1;
    name = (unsigned char*) malloc(256);
    name[0] = '\0';

    /* start reading the name to determine sizes, etc */
    while ( *read != 0 )
    {
        if ( *read >= 192 )
        {
            offset = ( (*read) << 8 ) + *( read + 1 ) - 49152;
            read = buf + offset - 1;
            jump = 1; // skip forward flag
        }
        else
        {
            name[p++] = *read;
        }

        read = read + 1;
		
		/* increment */
        if (jump == 0)
        {
            *count = *count + 1;
        }
    }

    name[p] = '\0'; // end of the name
	/* increment */
    if ( jump == 1 )
    {
        *count = *count + 1;
    }

    /* actually convert the name */
    for (i = 0; i < (int) strlen((const char*) name); i++)
    {
        p = name[i];
        for (j = 0; j < (int) p; j++)
        {
            name[i] = name[ i + 1 ];
            i = i + 1;
        }
        name[i] = '.';
    }
    name[ i - 1 ] = '\0';
    return (char *) name;
}

/* change the domain from www.example.com to the format the server expects
	eg 3www7example3com */
void set_domain( unsigned char* dns, unsigned char* hostp )
{
    int lock = 0, i;
    char host[32];
    strcpy( host, (char*) hostp );
    strcat( (char*) host, "." );

    for ( i = 0; i < (int) strlen( (char*) host ); ++i )
    {
        if ( host[i] == '.' )
        {
            *dns++ = i - lock;
			for( ; lock < i; ++lock )
            {
                *dns++ = host[lock];
            }
            ++lock;
        }
    }
    *dns++ = '\0';
}

/* parse the answers from the server into human readable format */
void parse_ans( char *buf, int qname_len )
{
    dns_header_t * dnshdr = NULL;
    int i,j,stop;
	i = j = stop = 0;
    char *read = NULL;//, *qname = NULL;
    dnshdr = (dns_header_t*) buf; // point the header to a buffer we can use
	
	/* info about response ... */
	printf( "Records in question section: %d\n", ntohs( dnshdr->q_count ) );
	printf( "Records in answer section: %d\n", ntohs( dnshdr->ans_count ) );
	printf( "Records in authority section: %d\n", ntohs( dnshdr->auth_count ) );
	printf( "Records in additional section: %d\n", ntohs( dnshdr->add_count ) );
	
	question_t *qinfo;
	qinfo = (question_t*) ( buf + sizeof( dns_header_t ) + qname_len );
	
	printf( "Query Type: %d\n", ntohs( qinfo->qtype ) );
	printf( "Query Class: %d\n", ntohs( qinfo->qclass ) );
	
	if( ntohs( dnshdr->ans_count <= 0 ) )
	{
		printf( "Error because we cannot resolve all requests.");
		return;
	}
	/* parse answers from the DNS server */
    read = (buf + sizeof(dns_header_t) + qname_len + sizeof(question_t));
    for ( i = 0; i < ntohs( dnshdr->ans_count ); i++ )
    {
		printf( "Answer record %d out of %d\n", i, ntohs( dnshdr->ans_count ) );
		ans[i].name = (unsigned char*) read_name( (unsigned char*) read, 
			(unsigned char*) buf, &stop );
        read = read + stop;

        ans[i].resource = (res_data_t*) (read);
        read = read + sizeof(res_data_t);
		
		printf( "Type: %d\n", ntohs( ans[i].resource->type ) );
		printf( "Class: %d\n", ntohs( ans[i].resource->_class ) );
		
		printf( "Host Name: %s\n", ans[i].name );
		/* normal resolution/response */
        if ( ntohs( ans[i].resource->type ) == T_A ) // typical IPv4
        {
            ans[i].rdata = (unsigned char*) malloc( ntohs( ans[i].resource->data_len ) );

            for ( j = 0; j < ntohs( ans[i].resource->data_len ); j++ )
            {
                ans[i].rdata[j] = read[j];
            }

            ans[i].rdata[ ntohs( ans[i].resource->data_len ) ] = '\0';
			struct sockaddr_in tmp;
			long *r;
			r = (long*) ans[i].rdata;
			tmp.sin_addr.s_addr = (*r);
			printf( "IP from server: %s\n", inet_ntoa( tmp.sin_addr ) );
            read = read + ntohs( ans[i].resource->data_len );
        }
        else // other type of response, just get some data from it
        {
            ans[i].rdata = (unsigned char*) read_name( (unsigned char*) read, (unsigned char*) buf, &stop );
            read = read + stop;
        }
    }
}

/* take the answer from the server (after parsing it)
	and print it out to the terminal */
void print_ans( int ans_count, char *str )
{
    int i = 0;
    struct sockaddr_in a;

    printf("\nNum Answers: %d\nAnswer Records -- \n", ans_count);
    for (i = 0; i < ans_count; i++)
    {
        printf("Host Name: %s ", ans[i].name);

        if ( ntohs( ans[i].resource->type ) == T_A )          //IPv4 address
        {
            long *p;
            p = (long*) ans[i].rdata;
            a.sin_addr.s_addr = (*p);
            printf( " -- IP Address from server: %s", inet_ntoa( a.sin_addr ) ); // convert addr and print
            strcpy( str, inet_ntoa( a.sin_addr) );
            break;
        }
		
		/* check for an alias and manage that too */
        if ( ntohs(ans[i].resource->type ) == 5 )
        {
            printf(" -- Alias: %s", ans[i].rdata);
        }
        printf("\n");
    }
}
