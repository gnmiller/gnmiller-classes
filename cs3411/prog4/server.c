/*
 * The server side of the card matching game application.
 * Serves the board and score in an infinite loop to the clients.
 * The data is written as quickly as the application may run and using 
 * the flag MSG_DONTWAIT in the calls to recv() the calling process does
 * not block waiting for the client to call send() or write().
 * 
 * @author Greg Miller
 * @author gnmiller
 * 
 * CS3411
 * Fall 2014
 * Program 4
 * Resubmit 2
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#define DEFAULT_PORT 50001

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[]){
    int sockfd, pfd[2], portno, status, score[2], guess[52];
    socklen_t clilen;
    char board[52];
    struct sockaddr_in serv_addr, p1_addr, p2_addr;
	 
	 /* open the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        error("ERROR opening socket");
		exit(1);
	}
    memset( &serv_addr, 0, sizeof(serv_addr));	 
	if( argc > 2 ) portno = atoi(argv[1]);
	else portno = DEFAULT_PORT;
	 
	/* setup info for server */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){ 
		perror( "Error binding socket:");
		exit(1);
	}
	/* start listening */
	printf("Server starting ...\n");
    listen(sockfd,5);
	printf("Server now listening on %d ...\n", portno);
    clilen = sizeof(p1_addr);
	
	/* wait for clients */
	printf("Waiting for clients to connect ... \n");
    pfd[0] = accept( sockfd, (struct sockaddr *) &p1_addr, &clilen );
	printf("Client 1 connected ...\nWaiting for client 2 ... \n");
	pfd[1] = accept( sockfd, (struct sockaddr *) &p2_addr, &clilen );
	printf("Client 2 connected ...\nStarting the game ...\n");
	
    if( pfd[0] < 0 ){
		perror("Error opening player 1 socket:");
		exit(1);		
	}
	if( pfd[1] < 0 ){
		perror("Error opening player 2 socket:");
		exit(1);
	}
	
	/* start the game */
	int i, index, in[2];
	char out[53];
	for(i = 0; i < 52; ++i) board[i] = 0;
	for(i = 0; i < 52; ++i) guess[i] = 0;
	score[0] = score[1] = 0;
	/* initialize the board, run with --no-debug to use a real board */
	if( argc > 1 && strcmp(argv[1], "--no-debug") ){
		for(i = 0; i < 26; ++i){
			char c = i+65; // 65 == A
		
			index = rand()%52;
			while( board[ index ] != 0 ) index = rand()%52;
			board[ index ] = c;
		
			index = rand()%52;
			while( board[ index ] != 0 ) index = rand()%52;
			board[ index ] = c;
		}
	} else strcpy(board, "aabbccddeeffgghhiijjkkllmmnnooppqqrrssttuuvvwwxxyyzz");
	   
	/* write initial data to clients */
	status = write( pfd[0], out, sizeof(char) * 53 );
	if( status < 0 ){
		perror("read error");
		exit(1);
	}
	status = write( pfd[1], out, sizeof(char) * 53 );
	if( status < 0 ){
		perror("read error");
		exit(1);
	}
	status = write( pfd[0], &score[0], sizeof(int) );
	if( status < 0 ){
		perror("read error");
		exit(1);
	}
	status = write( pfd[0], &score[1], sizeof(int) );
	if( status < 0 ){
		perror("read error");
		exit(1);
	}
	status = write( pfd[1], &score[0], sizeof(int) );
	if( status < 0 ){
		perror("read error");
		exit(1);
	}
	status = write( pfd[1], &score[1], sizeof(int) );
	if( status < 0 ){
		perror("read error");
		exit(1);
	}
	
	/* begin looping and playing the game */
	/* note that MSG_DONTWAIT is used to attain the specified "rapid-fire game */
	while( score[0] != 14 && score[1] != 14 ){
		if( score[0] == 13 && score[1] == 13 ) break;
		
		/* check if player 1 sent a guess */
		in[0] = in[1] = -1;
		status = recv( pfd[0], in, sizeof(int) * 2, MSG_DONTWAIT );
		if( in[0] != -1 && in[1] != -1 ){
			printf("Guess received from player 1 (%d,%d).\n",in[0],in[1]);
			if( board[ in[0] ] == board[ in[1] ] ){
				printf("Player 1 match\n");
				if( guess[ in[0] ] != 2 && guess[ in[1] ] != 2 ) ++score[0];				
				guess[ in[0] ] = 2;
				guess[ in[1] ] = 2;
				/* build and send the board */
				for( i = 0; i < 52; ++i ){
						if( guess[i] == 0 ){
						out[i] = '.';
					} else if( guess[i] == 2 ){
						out[i] = board[i];
					} else {
						out[i] = ' ';
					}
				} out[52] = '\0';
				status = write( pfd[0], out, sizeof(char) * 53 );
				if( status < 0 ){
					perror("read error");
					exit(1);
				}
				status = write( pfd[0], &score[0], sizeof(int) );
				if( status < 0 ){
					perror("read error");
					exit(1);
				}
				status = write( pfd[0], &score[1], sizeof(int) );
				if( status < 0 ){
					perror("read error");
					exit(1);
				}
			} else{
				printf("Player 1 no match\n");
				if( guess[ in[0] ] == 0 ) guess[ in[0] ] = 1;
				if( guess[ in[1] ] == 0 ) guess[ in[1] ] = 1;
				/* build and send the board */
				for( i = 0; i < 52; ++i ){
						if( guess[i] == 0 ){
						out[i] = '.';
					} else if( guess[i] == 2 ){
						out[i] = board[i];
					} else {
						out[i] = ' ';
					}
				} out[52] = '\0';
				status = write( pfd[0], out, sizeof(char) * 53 );
				if( status < 0 ){
					perror("read error");
					exit(1);
				}
				status = write( pfd[0], &score[0], sizeof(int) );
				if( status < 0 ){
					perror("read error");
					exit(1);
				}
				status = write( pfd[0], &score[1], sizeof(int) );
				if( status < 0 ){
					perror("read error");
					exit(1);
				}
			}
		} 
		
		/* check if player 2 sent a guess */
		in[0] = in[1] = -1;
		status = recv( pfd[1], in, sizeof(int) * 2, MSG_DONTWAIT );
		if( in[0] != -1 && in[1] != -1 ){
			printf("Guess from player 2 (%d,%d).\n",in[0],in[1]);
			if( board[ in[0] ] == board[ in[1] ] ){
				printf("Player 2 match\n");
				if( guess[ in[0] ] != 2 && guess[ in[1] ] != 2 ) ++score[1];				
				guess[ in[0] ] = 2;
				guess[ in[1] ] = 2;
				/* build and send the board */
				for( i = 0; i < 52; ++i ){
						if( guess[i] == 0 ){
						out[i] = '.';
					} else if( guess[i] == 2 ){
						out[i] = board[i];
					} else {
						out[i] = ' ';
					}
				} out[52] = '\0';
				status = write( pfd[1], out, sizeof(char) * 53 );
				if( status < 0 ){
					perror("read error");
					exit(1);
				}
				status = write( pfd[1], &score[1], sizeof(int) );
				if( status < 0 ){
					perror("read error");
					exit(1);
				}
				status = write( pfd[1], &score[0], sizeof(int) );
				if( status < 0 ){
					perror("read error");
					exit(1);
				}
			} else{
				printf("Player 2 no match\n");
				if( guess[ in[0] ] == 0 ) guess[ in[0] ] = 1;
				if( guess[ in[1] ] == 0 ) guess[ in[1] ] = 1;
				/* build and send the board */
				for( i = 0; i < 52; ++i ){
						if( guess[i] == 0 ){
						out[i] = '.';
					} else if( guess[i] == 2 ){
						out[i] = board[i];
					} else {
						out[i] = ' ';
					}
				} out[52] = '\0';
				status = write( pfd[1], out, sizeof(char) * 53 );
				if( status < 0 ){
					perror("read error");
					exit(1);
				}
				status = write( pfd[1], &score[1], sizeof(int) );
				if( status < 0 ){
					perror("read error");
					exit(1);
				}
				status = write( pfd[1], &score[0], sizeof(int) );
				if( status < 0 ){
					perror("read error");
					exit(1);
				}
			}
		} 
	}
	/* write initial data to clients */
	status = write( pfd[0], out, sizeof(char) * 53 );
	if( status < 0 ){
		perror("read error");
		exit(1);
	}
	status = write( pfd[1], out, sizeof(char) * 53 );
	if( status < 0 ){
		perror("read error");
		exit(1);
	}
	status = write( pfd[0], &score[0], sizeof(int) );
	if( status < 0 ){
		perror("read error");
		exit(1);
	}
	status = write( pfd[0], &score[1], sizeof(int) );
	if( status < 0 ){
		perror("read error");
		exit(1);
	}
	status = write( pfd[1], &score[0], sizeof(int) );
	if( status < 0 ){
		perror("read error");
		exit(1);
	}
	status = write( pfd[1], &score[1], sizeof(int) );
	if( status < 0 ){
		perror("read error");
		exit(1);
	}
	/* clean up the sockets */
	close( sockfd );
	close( pfd[0] );
	close( pfd[1] );
    return 0; 
}
