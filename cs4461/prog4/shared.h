#include "unreliable.h"
/* Put the prototypes here for any functions here which you want to be able
   to use by both client.c and server.c. */

// the UDP port users will be connecting to on server
#define SERVERPORT "6666"

/* info for packet parsing */
#define PACKET -1
#define SYN 0
#define ACK 1
#define FIN 2
#define FIN_SEQ 257
#define SYN_SEQ 0
#define ACK_SEQ 258

/* other generic macros */
#define YES 1
#define NO 0
// max message is 1024 thus
// 1 + 0 + 0 + 32 * 5 + 255 = 455 MAX CHECKSUM
// max length is then 13 (3 int + 5 space + up to 4 seq + 1 msg) + max checksum length (3)
// we can fit the entire packet in 16 but to be safe just use 32 in case math is wrong
#define PACKET_SIZE 32

/* info for timing out a connection */
#define TIMEOUT_MS 500
#define TIMEOUT_S 1
#define MS_CONV 1000
#define WINDOW 3

/* packet struct */
typedef struct {
	int syn;
	int ack;
	int fin;
	int seq;
	int checksum;
	char msg[2];
	char xmit[1028];
} packet;

/* functions */
int parsePacket( char *s, packet *p );
void fillPacket( packet *p, char msg, int type, int seq );
int checksum( char *msg );
int getIP();
