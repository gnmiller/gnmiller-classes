/*
 * The client side of the card game application.
 * In order to avoid crashes and errant errors when the
 * game ends the server closes the sockets and the client detects
 * this. If this occurs it is a known loss.
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
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

int main(int argc, char *argv[]){
	
    int sockfd, portno, status, i, score[2];
    struct sockaddr_in serv_addr;
    struct hostent *server;
	char board[52];
	
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        perror("ERROR opening socket");
		exit(1);
	}
	
	/* get host name */
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    memset( &serv_addr, 0, sizeof(serv_addr) );
    serv_addr.sin_family = AF_INET;
	/* copy addr info into the struct */
    strcpy( (char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr );
    serv_addr.sin_port = htons(portno);
	/* attempt to connect */
    status = connect( sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr) ) ;
	if( status < 0 ){
        perror("Error connecting to server:");
		exit(1);
	}
	int in[2];
	/* socket ready for reading and writing */
	printf("Welcome to the game!\n");
	int flag = 0;
	int flag2 = 0;
	printf("\n");
	/* start playing the game */
	for( ;; ){
		/* read data from the socket */
		status = recv( sockfd, board, sizeof(char)*53, 0 );
		if( status <= 0 ) flag2 = 1;
		status = recv( sockfd, &score[0], sizeof(int), 0 );
		if( status <= 0 ) flag2 = 1;
		status = recv( sockfd, &score[1], sizeof(int), 0 );
		if( status <= 0 ) flag2 = 1;
		if( flag2 == 1 ) score[1] = 14;
		
		/* is the game over? */
		if( score[0] == 14 ) break;
		if( score[1] == 14 ) break;
		if( score[0] == 13 && score[1] == 13 ) break;
		
		/* read guesses and print some data */
		if( flag == 0 ) {
			printf("Current Board:");
			for(i = 0; i < 52; ++i) printf(".");
			printf("\n");
		}
		if( flag == 1 ) printf("Current Board: %s\n", board);
		flag = 1;
		printf("Current Score: (You) %d, %d\n",score[0],score[1]);
		printf("\n\nNote: entering already matched numbers is a lost guess.\n");
		printf("Enter your first guess [0-51]:\n");
		scanf( "%d", &in[0] );
		while( in[0] < 0 || in[0] > 51 ){
			printf("Invalid input, try again.\n");
			scanf( "%d", &in[0] );
		}
		printf("Enter your second guess [0-51]\n");
		scanf( "%d", &in[1] );
		while( in[1] < 0 || in[1] > 51 || in[0] == in[1] ){
			printf("Invalid input, try again.\n");
			scanf( "%d", &in[1] );
		}
		
		/* write the guesses out to the server */
		status = write( sockfd, in, sizeof(int) * 2);
		if( status < 0 ){
			perror("read error");
			exit(1);
		}
	}
	/* determine victory and clean up */
	if( score[0] == 14 ) printf("Congratulations! You win!\n");
	else if( score[1] == 14 ) printf("Sorry, you lost :(\n");
	else printf("The game was a draw, better luck next time.\n");
    close(sockfd);
    exit(1);
}