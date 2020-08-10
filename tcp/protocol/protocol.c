#include "protocol.h"

int protocol_packet_get_seq_id(protocol_packet_t* packet){
	if(packet == NULL)
		return ARG_NULL;

	return packet->header.seq_id;
}

int protocol_packet_get_packet_type(protocol_packet_t* packet){
	if(packet == NULL)
		return ARG_NULL;
	return packet->header.packet_type;
}

int protocol_packet_get_body_len(protocol_packet_t* packet){
	if(packet == NULL)
		return ARG_NULL;
	return packet->header.body_len;
}

int protocol_packet_set_seq_id(protocol_packet_t* packet, int val){
	if(packet == NULL)
		return ARG_NULL;

	packet->header.seq_id = val;
	return 0;
}

int protocol_packet_set_packet_type(protocol_packet_t* packet, int val){
	if(packet == NULL)
		return ARG_NULL;
	packet->header.packet_type = val;
	return 0;
}

int protocol_packet_set_body_len(protocol_packet_t* packet,int val){
	if(packet == NULL)
		return ARG_NULL;
	packet->header.body_len = val;
	return 0;
}

protocol_packet_t* protocol_packet_init(){
	protocol_packet_t* packet = malloc(sizeof(protocol_packet_t));
	if(packet == NULL)
		return NULL;
	
	if(memset(packet,0,sizeof(protocol_packet_t)) ==0)
		return NULL;
	
	return packet;
}
int protocol_packet_destroy(protocol_packet_t* packet){
	if(packet == NULL)
		return ARG_NULL;
	free(packet);
	return 0;
}


