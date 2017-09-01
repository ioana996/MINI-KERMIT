#ifndef LIB
#define LIB

#define MAX_L 		243
#define TIME_MS 	5000

#define MAXL 		0
#define TIME 		1
#define NPAD		2
#define PADC		3
#define EOL			4
#define QCTL		5
#define QBIN		6
#define CHKT		7
#define REPT		8
#define CAPA 		9
#define R 			10

typedef struct {
    int len;
    char payload[1400];
} msg;

typedef struct kermit {
	unsigned char soh;
	unsigned char len;
	unsigned char seq;
	unsigned char type;
	unsigned char data[MAX_L];
	unsigned short check;
	unsigned char mark;
}  __attribute__ (( packed )) KermitFrame;

typedef struct sendInit {
	unsigned char fields[11];
} __attribute__ (( packed )) SendInitFrame;

void init(char* remote, int remote_port);
void set_local_port(int port);
void set_remote(char* ip, int port);
int send_message(const msg* m);
int recv_message(msg* r);
msg* receive_message_timeout(int timeout); //timeout in milliseconds
unsigned short crc16_ccitt(const void *buf, int len);

#endif

