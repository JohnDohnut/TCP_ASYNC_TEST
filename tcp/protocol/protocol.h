#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BODY_LEN 1024
#define HEADER_SIZE 12

typedef struct protocol_header_s packet_header_t;
struct protocol_header_s{
	int seq_id;
	int packet_type;
	int body_len;
};

typedef struct protocol_packet_s protocol_packet_t;
struct protocol_packet_s{
	packet_header_t header;
	char body[MAX_BODY_LEN];
};


protocol_packet_t* protocol_packet_init();
int protocol_packet_get_seq_id(protocol_packet_t* packet);
int protocol_packet_destroy(protocol_packet_t* packet);


#endif
