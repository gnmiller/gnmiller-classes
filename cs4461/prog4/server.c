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
#include <signal.h>

#include "unreliable.h"
#include "shared.h"


#define MAXBUFLEN 100

int main(void)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
	
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
	        perror("server: socket");
	        continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind socket\n");
        return 2;
    }
    freeaddrinfo(servinfo);
	char *recv_msg, *msg;
	recv_msg = malloc( sizeof( char ) * 2048 );
	msg = malloc( sizeof( char ) * 2048 );
	packet *pack;
	pack = malloc( sizeof( packet ) );
	int flag[2048],count,connected,status;
	count = 0;
	for(int i=0;i<2048;++i)flag[i]=0;
	
	/* timeout on the socket .. */
	struct timeval tv;
	tv.tv_sec = TIMEOUT_S;
	tv.tv_usec = TIMEOUT_MS * MS_CONV;
	if ( setsockopt( sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0 )
	{
		perror("Error");
	}

    while( 1 )
    {
		printf("Waiting for something to read...\n");
		addr_len = sizeof their_addr;
		
		/* recv a packet on the socket */
		if ((numbytes = recvfrom(sockfd, (void*)recv_msg, sizeof( char ) * PACKET_SIZE , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
	    {
			if( errno != EAGAIN || errno != EWOULDBLOCK )
			{
				perror("recvfrom");
				exit(1);
			}
			else
			{
				printf( "Server has timed out\n" );
				continue;
			}
	    }
	    printf( "Server received: %s\n", recv_msg );
	    /* checksum and struct */
		if( parsePacket( recv_msg, pack ) )
		{
			printf( "Server received corrupt packet.\n" );
			continue;
		} // good packet
		
		if( pack->syn ) // syn
		{
			printf( "SYN PACKET\n" );
			if( connected ) // reset
			{
				printf( "SYN: Prev connection discarded.\n" );
				for(int i=0;i<2048;++i)flag[i]=0;
				count = 0;
				memset( msg, 0, 2048 );
				memset( pack, 0, sizeof( packet ) );
			}
			
			/* transmit the ack */
			printf( "SYN: Handshake\n" );
			packet *ack_pack = malloc( sizeof( packet ) );
			fillPacket( ack_pack, 'X', ACK, SYN_SEQ );
			status = 0;
			connected = 1;
			status = unreliable_sendto( sockfd, (void*)ack_pack->xmit, sizeof( char ) * PACKET_SIZE, 0, (struct sockaddr *)&their_addr, addr_len );
			if( status < 0 )
			{
				printf( "SYN: sendto() fail\n" );
				exit( EXIT_FAILURE );
			}
			free(ack_pack);
		}
		else if( pack->ack ) // ack
		{
			// shouldnt ever transmit an ack from client ..
		}
		else if( pack->fin ) // fin
		{
			printf( "FIN PACKET\n" );			
			
			/* transmit the ack */
			printf( "FIN: Acknowledging\n" );
			packet *ack_pack = malloc( sizeof( packet ) );
			fillPacket( ack_pack, 'X', ACK, FIN_SEQ );
			status = 0;
			status = unreliable_sendto( sockfd, (void*)ack_pack->xmit, sizeof( char ) * PACKET_SIZE, 0, (struct sockaddr *)&their_addr, addr_len );
			if( status < 0 )
			{
				printf( "FIN: sendto() fail\n" );
				exit( EXIT_FAILURE );
			}
			free(ack_pack);
			
			if( connected )
				printf( "FIN: The completed message received from the client is: %s\n", msg );
			
			sleep( 3 ); // wait to read the message, then reset
			
			/* reset */
			connected = 0;
			printf( "FIN: resetting the connection\n" );
			for(int i=0;i<2048;++i)flag[i]=0;
			memset( msg, 0, 2048 );
			memset( pack, 0, sizeof( packet ) );
			count = 0;
		}
		else // generic message packet
		{
			printf( "PACKET RECEIVED\n" );
			if( flag[pack->seq] )
			{
				printf( "PACKET RECEIVED:  detected alread recvd and ackd packet.\nRetransmitting the ack." );
				/* transmit the ack */
				packet *ack_pack = malloc( sizeof( packet ) );
				fillPacket( ack_pack, 'X', ACK, pack->seq );
				status = 0;
				unreliable_sendto( sockfd, (void*)ack_pack->xmit, sizeof( char ) * PACKET_SIZE, 0, (struct sockaddr *)&their_addr, addr_len );
				if( status < 0 )
				{
					printf( "PACKET RECEIVED:  sendto() fail\n" );
					exit( EXIT_FAILURE );
				}
				free(ack_pack);				
			}
			else
			{
				flag[pack->seq] = 1; // flag we got this message already
				msg[pack->seq] = pack->msg[0]; // copy the msg
				printf( "PACKET RECEIVED: Number: %d\nPACKET RECEIVED: Payload: %c\nPACKET RECEIVED: Sequence: %d\n",
						count, pack->msg[0], pack->seq );
				++count;
				
				/* transmit the ack */
				packet *ack_pack = malloc( sizeof( packet ) );
				fillPacket( ack_pack, 'X', ACK, pack->seq );
				status = 0;
				unreliable_sendto( sockfd, (void*)ack_pack->xmit, sizeof( char ) * PACKET_SIZE, 0, (struct sockaddr *)&their_addr, addr_len );
				if( status < 0 )
				{
					printf( "PACKET: RECEIVED: sendto() fail\n" );
					exit( EXIT_FAILURE );
				}
				free(ack_pack);
			}
		}
	   
    }

    close(sockfd);

    return 0;
}

