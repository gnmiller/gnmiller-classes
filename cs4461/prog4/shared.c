#include "unreliable.h"
#include "shared.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

/* Put any functions here which you want to be
   able to use by both client.c and server.c. */

/* takes a packet and generates the transmit string for it
 * the packet must be initialized prior (all but transmit)
 */
void fillPacket( packet *p, char msg, int type, int seq )
{	
	if( type == SYN )
	{
		p->syn = YES;
		p->ack = NO;
		p->fin = NO;
		p->seq = SYN_SEQ;
		strcpy( p->msg, "X" );
	}
	else if ( type == ACK )
	{
		p->syn = NO;
		p->ack = YES;
		p->fin = NO;
		p->seq = seq;
		strcpy( p->msg, "X" );
	}
	else if ( type == FIN )
	{
		p->syn = NO;
		p->ack = NO;
		p->fin = YES;
		p->seq = FIN_SEQ;
		strcpy( p->msg, "X" );
	}
	else
	{
		p->syn = NO;
		p->ack = NO;
		p->fin = NO;
		p->seq = seq;
		strcpy( p->msg, &msg );
		p->msg[1] = '\0';
	}
	
	/* calc the checksum */
	sprintf( p->xmit, "%d %d %d %d %s ", p->syn, p->fin, p->ack, p->seq, p->msg );
	p->checksum = checksum( p->xmit );
	
	/* stick the checksum to the packet */
	char buf[1024];
	sprintf( buf, "%d", p->checksum );
	strcat( p->xmit, buf );
	
	return;
}

/**
 * Fill the packet p based on the string s 
 */
int parsePacket( char *s, packet *p )
{
	int syn, fin, ack, seq, chksum;
	char sp[5], msg; 
	msg = 0;
	if( sscanf( s, "%d%c%d%c%d%c%d%c%c%c%d",
		&syn, &sp[1], &fin, &sp[2], &ack, &sp[3], &seq, &sp[4], &msg, &sp[5], &chksum ) != 11 )
	{
		return -1;
	}
	else // successful parse
	{
		if( syn )
		{
			fillPacket( p, 'X', SYN, SYN_SEQ );
		}
		else if( fin )
		{
			fillPacket( p, 'X', FIN, FIN_SEQ );
		}
		else if( ack )
		{
			fillPacket( p, 'X', ACK, seq );
		}
		else
		{
			fillPacket( p, msg, PACKET, seq );
		}
		
		/* check the checksum 
			fillPacket() puts the checksum in for us */
		if( p->checksum != chksum )
		{
			return -2;
		}
		
		return 0;
	}
}

int checksum( char* msg )
{
	int ret = 0;
	for( int i = 0; i < strlen( msg ); ++i )
	{
		ret += (int)msg[i];
	}
	return ret;
}