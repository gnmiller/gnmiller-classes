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
 * File: helper.cc
 * 
 * Contains the definitions for the prototypes specified in helper.h
 */

#include "helper.h"
#include <string.h>
#include <cstdlib>

/* Various Node constructors follow */
Node::Node(){};

Node::Node( char* d )
{
	data = (char*) malloc( strlen ( d ) * sizeof( char ) );
	strcpy( data, d );
};

IPNode::IPNode( char* d, char* d2 ) : Node( d )
{
	data2 = (char*) malloc( strlen( d2 ) * sizeof( char ) );
	strcpy( data2, d2 );
};

IPNode::IPNode( IPNode *i ) : Node( i->data )
{
	data2 = (char*) malloc( strlen( i->data2 ) * sizeof( char ) );
	strcpy( data2, i->data2 );
};


RNode::RNode( char* d, char *d2, int d3 ) : IPNode( d, d2 )
{
	o = d3;
}
/* end node constructors */

/**
 * Tests an IP address for validity.
 * ip: The IP to check
 * 
 * Validity is tested by first inspecting the number of
 * octets the IP contains. If there are not exactly 4 then
 * it is invalid.
 * 
 * Second it tests the values of each octet. Any value such
 * 0 < val < 255 is not true is invalid.
 * 
 * returns 0 in the case that ip is a valid IP, 0 otherwise.
 */
int check_ip( char* ip )
{
	int octets[4];
	int substr_c = 0;
	char *str = ip;
	while( ( str = strstr( str, "." ) ) )
	{
		++substr_c;
		++str;
	}
	if( substr_c != 3 )
	{
		return substr_c;
	}
	
	char str2[33];
	strcpy( str2, ip );
	octets[0] = atoi( strtok( str2, "." ) );
	for( int i = 1; i < 4; ++i )
	{
		octets[i] = atoi( strtok( NULL, "." ) );
		if( octets[i] > 255 || octets[i] < 0 )
		{
			return -1;
		}
	}
	return 0;
};