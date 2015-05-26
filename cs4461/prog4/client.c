// CS4461 - PUT STUDENT NAME HERE

/* Template code provided by Scott Kuhl and based on:
   http://www.beej.us/guide/bgnet/output/html/singlepage/bgnet.html
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include "unreliable.h"
#include "shared.h"

int main(int argc, char *argv[])
{
	int sockfd,msgLen;
	
	struct addrinfo hints, *servinfo, *p;
	int rv;

	if (argc != 3) {
		fprintf(stderr,"usage: %s hostname message\n", argv[0]);
		exit(1);
	}
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
		                     p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to bind socket\n");
		return 2;
	}

	/* 
	 * generate the packets to send 
	 */
	msgLen = strlen( argv[2] );
	char *message = malloc( sizeof( char ) * msgLen );
	if( message == NULL ) exit( EXIT_FAILURE );
	strcpy( message, argv[2] );
	packet *packets[ msgLen ];
	int ack[ msgLen ];
	for( int i = 0; i < msgLen; ++i )
	{
		packets[i] = malloc( sizeof( packet ) );
		memset( packets[i], 0, sizeof( packet ) );
		fillPacket( packets[i], message[i], PACKET, i ); // -1 is for a normal packet
		ack[i] = 0;
	} ack[0] = 0;
	free(message);
	
	/* init data */
	int count, connected, ret, wait_count, fin_ack;
	char *recv_msg;
	connected = ret = wait_count = fin_ack = count = 0;
	
	recv_msg = malloc( sizeof( char ) * 2048 );
	packet *pack = malloc( sizeof( packet ) );
	if( pack == NULL ) exit( EXIT_FAILURE );
	if( recv_msg == NULL ) exit( EXIT_FAILURE );

	/* timeout on the socket .. 
		this block was retrieved from a stackoverflow post */
	struct timeval tv;
	tv.tv_sec = TIMEOUT_S;
	tv.tv_usec = TIMEOUT_MS * MS_CONV; //150ms
	if ( setsockopt( sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0 )
	{
		perror("Error");
	}
	
	/* loop to send message */
	int timeout = 0;
	packet *w[3];
	
	/* 'init' the window to prevent errors */
	for( int i = 0; i < WINDOW; ++i )
		w[i] = packets[0];
	
	while( count < msgLen )
	{
		if( ack[count] == 1 ) 
		{
			++count;
			continue;
		} // prevent inf looping ...
		
		if(timeout >= 50) // make sure we dont loop forever if something bad happens
		{
			printf( "Connection appears dead, client terminating!\n" );
			free(recv_msg);
			free(pack);
			freeaddrinfo(servinfo);
			exit( EXIT_FAILURE );
		}
		
		/* attempt to "connect" and handshake with the server */
		while( !connected )
		{
			/* construct and send syn packet */
			packet *syn_pack = malloc( sizeof( packet ) );
			//memset( syn_pack, 0, sizeof( packet ) );
			fillPacket( syn_pack, 'X', SYN, SYN_SEQ );
			unreliable_sendto( sockfd, (void*)syn_pack->xmit, sizeof( char ) * PACKET_SIZE, 0, p->ai_addr, p->ai_addrlen );
			printf( "CLIENT: Handshaking.\n" );
			free(syn_pack);
			
			/* get an ack to handshake */
			memset( recv_msg, 0, 2048 );
			ret = -1;
			errno = 0;
			ret = recvfrom( sockfd, (void*)recv_msg, sizeof( char ) * PACKET_SIZE , 0, p->ai_addr, &p->ai_addrlen );
			if( ret > 0 ) // got some data off the socket
			{
				memset( pack, 0, sizeof( packet ) );
				
				/* parsePacket parses the msg into tokens (syn,fin,etc) and then performs checksum matching */
				if( parsePacket( recv_msg, pack ) < 0 ) // fill pack with data based on recv_msg
				{
					printf( "CLIENT: Corrupt packet.\n" );
					continue;
				}
				
				/* handshake .. */
				if( pack->ack && pack->seq == SYN_SEQ )
				{
					printf( "CLIENT: Handshake completed.\n" );
					connected = 1;
				}
			}
			else
			{
				if( errno != EAGAIN || errno != EWOULDBLOCK )
				{
					perror("recvfrom");
					exit(1);
				}
				else
				{
					printf( "CLIENT: HANDSHAKE timeout\n" );
					++timeout;
					continue;
				}
			}
			sleep( 1 ); // sleep here because if we go to too fast it might confuse the server
						// we send too many syn and can never get an ack back fast enough
		} // connected
		
		/* start sending the message */
		printf( "WINDOW(seq): %d, %d, %d\n", w[0]->seq, w[1]->seq, w[2]->seq );
		
		if( wait_count < WINDOW )
		{
			unreliable_sendto( sockfd, (void*)packets[count]->xmit, sizeof( char ) * PACKET_SIZE, 0, p->ai_addr, p->ai_addrlen );
			++count;
			if( count >= msgLen ) break;
			printf( "CLIENT(%d): Transmitting packet(seq) %d with payload %s to server.\n", count,
					packets[count]->seq, packets[count]->msg );
			w[wait_count] = packets[count];
			++wait_count;
		}
		/* send initial packets */
		if( count < WINDOW ) continue;
		
		/* update the window 
			keeps packets from getting stuck in low slots */
		for( int i = 0; i < wait_count; ++i )
			if( ack[ w[i]->seq ] ) // has this window member been ack'd?
				for( int j = 0; j < count; ++j )
					if( !ack[j] ) // search for a sent, unack'd
						w[i] = packets[j];
		
		/* recv some acks */
		for( int i = 0; i < wait_count; ++i )
		{
			/* recv .. */
			memset( recv_msg, 0, 2048 );
			ret = -1;
			errno = 0;
			ret = recvfrom( sockfd, (void*)recv_msg, sizeof( char ) * PACKET_SIZE , 0, p->ai_addr, &p->ai_addrlen );
			if( ret > 0 ) // got some data off the socket
			{
				--wait_count;
				printf( "CLIENT: Received %s from server\n", recv_msg );
				/* checksum and populate struct */
				memset( pack, 0, sizeof( packet ) );
				if( parsePacket( recv_msg, pack ) < 0 )
				{
					printf( "CLIENT: Corrupt packet.\n" );
					continue;
				}
				
				/* mark the ack */
				if( pack->seq > count ) // where did this ack come from?
				{
					printf( "CLIENT: Received an ack for a packet I have yet to transmit. It will be discarded.\n" );
					continue;
				}
				else
				{
					if( ack[ pack->seq ] == 0 )
					{
						printf( "CLIENT: Received ack for packet: %d\n", pack->seq );
						ack[ pack->seq ] = 1;
					}
				}
			}
			else // timed out
			{
				if( errno != EAGAIN || errno != EWOULDBLOCK )
				{
					perror("recvfrom");
					exit(1);
				}
				else
				{
					printf( "CLIENT: timeout (initial)\n" );
					++timeout;
					continue;
				}
			}
		} // done trying to rceive acks
		
		/* resend from the window */
		for( int i = 0; i < wait_count; ++i )
		{
			if( ack[ ( w[i] )->seq] == 0 )
			{
				printf( "CLIENT: retransmitting packet from window: %d with payload: %s to the server\n",
						w[i]->seq, w[i]->msg );
				unreliable_sendto( sockfd, (void*)w[i]->xmit, sizeof( char ) * PACKET_SIZE, 0, p->ai_addr, p->ai_addrlen );
				// dont change window here since we didnt ack, or add a new packet
			}
		}
		
		/* try to resend stuff */
		for( int i = 0; i < count; ++i )
		{
			if( ack[i] != 1 && wait_count < WINDOW )
			{
				printf( "CLIENT: retransmitting packet: %d with payload: %s to the server\n",
						packets[i]->seq, packets[i]->msg );
				unreliable_sendto( sockfd, (void*)packets[i]->xmit, sizeof( char ) * PACKET_SIZE, 0, p->ai_addr, p->ai_addrlen );
				w[wait_count] = packets[i]; // store it in the window
				++wait_count; // inc window ptr
			}
			if( wait_count > WINDOW ) break;
		}
	}
	
	int done = 0;
	count = 0;	
	/* make sure we ack'd everything */	
	while( !done )
	{
		for( int i = 0; i < msgLen; ++i ) if( ack[i] == 0 ) done = 0; else done = 1;
		if( ack[count] == 0 ) // check if this guy has been ack'd
		{
			if( wait_count < WINDOW )
			{
				printf( "CLIENT: retransmitting packet: %d with payload: %s to the server\n",
						packets[count]->seq, packets[count]->msg );
				unreliable_sendto( sockfd, (void*)packets[count]->xmit, sizeof( char ) * PACKET_SIZE, 0, p->ai_addr, p->ai_addrlen );
				w[wait_count] = packets[count];
				++wait_count;
			} // cannot resend queue full
		} else ++count; // packet was ack'd
		if( count >= msgLen ) break;
		
		/* recv some acks */
		for( int i = 0; i < wait_count; ++i )
		{
			/* recv .. */
			memset( recv_msg, 0, 2048 );
			ret = -1;
			errno = 0;
			ret = recvfrom( sockfd, (void*)recv_msg, sizeof( char ) * PACKET_SIZE , 0, p->ai_addr, &p->ai_addrlen );
			if( ret > 0 ) // got some data off the socket
			{
				--wait_count;
				printf( "CLIENT: Received %s from server\n", recv_msg );
				/* checksum and populate struct */
				memset( pack, 0, sizeof( packet ) );
				if( parsePacket( recv_msg, pack ) < 0 )
				{
					printf( "CLIENT: Corrupt packet.\n" );
					continue;
				}
				if( ack[ pack->seq ] == 0 )
				{
					printf( "CLIENT: Received ack for packet: %d\n", pack->seq );
					ack[ pack->seq ] = 1;
				}
			}
			else // timed out
			{
				if( errno != EAGAIN || errno != EWOULDBLOCK )
				{
					perror("recvfrom");
					exit(1);
				}
				else
				{
					printf( "CLIENT: timeout (resend)\n" );
					++timeout;
					continue;
				}
			}
		} // done trying to rceive acks
		if( count == msgLen ) done = 1;
	}
	
	/* done sending make fin and send it */
	packet *fin_pack = malloc( sizeof( packet ) );
	if( fin_pack == NULL ) exit( EXIT_FAILURE );
	fillPacket( fin_pack, 'X', FIN, FIN_SEQ );
	memset( pack, 0, sizeof( packet ) );
	while( fin_ack == 0 )
	{
		if( timeout >= 50 ) break;
		if( fin_ack == 1 ) break;
		unreliable_sendto( sockfd, (void*)fin_pack->xmit, sizeof( packet ), 0, p->ai_addr, p->ai_addrlen );
		printf( "CLIENT: transmitting FIN to server\n" );
		
		/* recv .. */
		memset( recv_msg, 0, 2048 );
		ret = -1;
		errno = 0;
		sleep( 1 );
		ret = recvfrom( sockfd, (void*)recv_msg, sizeof( char ) * PACKET_SIZE , 0, p->ai_addr, &p->ai_addrlen );
		if( ret > 0 ) // got some data off the socket
		{
			printf( "CLIENT: Received %s from server\n", recv_msg );
			/* checksum and populate struct */
			memset( pack, 0, sizeof( packet ) );
			if( parsePacket( recv_msg, pack ) < 0 )
			{
				printf( "CLIENT: Corrupt packet.\n" );
				continue;
			}
			if( pack->ack != 1 || pack->seq != FIN_SEQ ) continue; // discard and try again
			printf( "CLIENT received ack for FIN\n" );
			if( pack->seq == FIN_SEQ )
			{
				fin_ack = 1;
			}
		}
		else // timed out
		{
			if( errno != EAGAIN || errno != EWOULDBLOCK )
			{
				perror("recvfrom");
				exit(1);
			}
			else
			{
				printf( "CLIENT: FIN timeout\n" );
				++timeout;
				continue;
			}
		}
	}
	free( fin_pack );	
	
	/* free the message array */
	for( int i = 0; i < msgLen; ++i )
	{
		free( packets[i] );
	}
	
	free(recv_msg);
	free(pack);
	freeaddrinfo(servinfo);
	close(sockfd);
	return 0;
}
