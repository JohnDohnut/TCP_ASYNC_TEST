#include "client.h"
/**
*@fn void client_set_msg
*@brief sets sample packet for test
*@param protocol_packet_t* 
*@return
*/
void client_set_msg(protocol_packet_t* packet){
	packet->header.seq_id ++;
	if(packet->header.packet_type == 0){
		packet->header.packet_type = 1;
		strncpy(packet->body, "aaaa",MAX_BODY_LEN);
	}
	else{
		packet->header.packet_type = 0;
		strncpy(packet->body, "bbbb",MAX_BODY_LEN);
	}
}
/**
*@fn 
*@brief 
*@param 
*@return
*/
int client_get_sockfd(client_t* client){
	if(client == NULL){
		return -1;
	}
	return client->sockfd;
}

struct sockaddr_in* client_get_addr(client_t* client){
	if(client==NULL){
		return NULL;
	}
	return &(client->addr);
}


int client_get_epoll_in_fd(client_t* client){
	if(client == NULL){
		return ARG_NULL;
	}
	return client->epoll_in_fd;
}

int client_get_epoll_out_fd(client_t* client){
	if(client == NULL){
		return ARG_NULL;
	}
	return client->epoll_out_fd;
}
/*
struct epoll_event* client_get_events(client_t* client){
	if(client == NULL){
		return NULL;	
	}
	return client->events;
}
*/
int client_check_fd (int fd){
	int error = 0;
	socklen_t err_len = sizeof( error);
	if( getsockopt( fd, SOL_SOCKET, SO_ERROR, &error, &err_len) < 0){
		return FD_ERR;
	}
	return NO_ERR;	
}

int client_set_fd_nonblock(int fd){
	int curr_flag = 0;
	/// get current flag set on fd.
	if( (curr_flag = fcntl(fd, F_GETFL, 0)) < 0){
		printf("	| ! Client : fcntl error on get flag\n");
		return FD_ERR;
	}
	/// set fd flag rv(current flag)|O_NONBLOCK
	if( fcntl(fd, F_SETFL, curr_flag | O_NONBLOCK) < 0){
		printf("	| ! Client : fcntl error on set flag\n");
		return FD_ERR;
	}
	return NO_ERR;
}

int client_connect(client_t* client){
	int i=0;
	int j=0;
	int write_bytes,read_bytes = 0;
	int write_rv,read_rv = 0;
	protocol_packet_t* send_packet = malloc(sizeof(protocol_packet_t));
	protocol_packet_t* recv_packet = malloc(sizeof(protocol_packet_t));
	memset(send_packet,0,sizeof(protocol_packet_t));
	memset(recv_packet,0,sizeof(protocol_packet_t));
	client_set_msg(send_packet);
	if(client == NULL)
		return CLIENT_NULL;
	int event_count = 0;
	while(j<10){
		protocol_packet_get_seq_id(recv_packet);
		if(client_check_fd(client->sockfd)){
			printf("	| ! Client : In client_conenct()... server fd error\n");
			return SERVER_NULL;
		}
		event_count = epoll_wait(client->epoll_out_fd, client->out_events, MAX_EVENTS, TIMEOUT);
		if(event_count < 0){
				///error
				printf("	| ! Server : In server_connect()... event_count < 0\n");
				return EPOLL_ERR;
			}
		else if (event_count == 0){
			printf("no out event\n");
			continue;
		}
		else{	printf("out event\n");
			for (i=0;i<event_count;i++){
				if(client->out_events[i].data.fd == client->sockfd){
					write_rv = write(client->sockfd, send_packet, sizeof(*send_packet));
					j++;
					printf("write_rv : %d, body : %s\n",write_rv,send_packet->body);
					if(write_rv < 0){
						if(write_rv == EAGAIN || write_rv == EWOULDBLOCK){
							return ERRNO_EAGAIN;
						}
						else if (write_rv == EINTR){
							printf("	| ! Client : In client_connect()... EINTR while write()\n");
							return INTERRUPT;
						}
						else {
							printf("	| ! Client : In client connect()... NEGATIVE_BYTES while write()\n");
							return NEGATIVE_BYTE;
						}
					}
					else if(write_rv == 0){
						printf("zero_byte\n");
						break;					
						///zero byte err
					}
					else{
						printf("	| @ Client : packet %dbytes sent, sent seq_id : %d\n",write_rv,send_packet->header.seq_id);
						client_set_msg(send_packet);
					}
					if(write_bytes == sizeof(protocol_packet_t)){
						printf("	| @ Client : packet sent seq_id : %d\n",send_packet->header.seq_id);
						write_bytes =0;
						client_set_msg(send_packet);

					}
							
				}
			}
		}
		event_count = epoll_wait(client->epoll_in_fd, client->in_events, MAX_EVENTS, TIMEOUT);
		if(event_count < 0){
				///error
				printf("	| ! Server : In server_connect()... event_count < 0\n");
				return EPOLL_ERR;
			}
		else if (event_count == 0){
			continue;
		}
		else{	printf("in event\n");
			for (i=0;i<event_count;i++){
				if(client->out_events[i].data.fd == client->sockfd){
					read_rv = read(client->sockfd, recv_packet+read_bytes, sizeof(protocol_packet_t));
					if(read_rv<0){
						///error handle	
					}		
					else if(read_rv == 0){
						///zero byte err
					}
					else{
						read_bytes += read_rv;
					}
					if(read_bytes == sizeof(protocol_packet_t)){
						printf("	| @ Client : packet recv seq_id : %d\n",recv_packet->header.seq_id);
						read_bytes = 0;
						memset(recv_packet,0,sizeof(protocol_packet_t));
					}
				}
			}
		}
		
	}
	return 0;
}

client_t* client_init(int argc, char** argv){
	/// error check
/*	if(error != NO_ERR){
		printf("	| ! Client : There was an error during previous process\n");
		return NULL;
	}*/
	/// main param check
	if(argc!=3){
		// *error = WRONG_MAIN_PARAM;
		printf("	| ! Client : In client_init()... wrong main parameter\n");
		return NULL;
	}
	/// client_t* malloc	
	client_t* client = malloc(sizeof(client_t));
	if(client == NULL){
		// *error = SERVER_NULL;
		printf("	| ! Client : In client_init()... malloc() failed\n");
		return NULL;
	}
	/// initialze client_t*
	if(memset(client,0,sizeof(client_t)) == NULL){
		// *error = UNKNOWN;
		printf("	| ! Client : In client_init()... memset() client_t failed\n");
		return NULL;
	}
	/// declare and initialize space for client addr
	char ip[MAX_MAIN_ARG_LEN];
	int port = -1;

	if(memset(ip,0,sizeof(ip)) == NULL){
		// *error = UNKNOWN;
		printf("	| ! Client : In client_init()... memset() ip buf failed\n");
	}
	strncpy(ip,argv[1],MAX_MAIN_ARG_LEN);
	port = atoi(argv[2]);

	/// socket() client
	if ( (client->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		// *error = SOCK_FAIL;
		printf("	| ! Client : In client_init()... socket() failed\n");
		return NULL;
	}	
	client->addr.sin_family = AF_INET;
	client->addr.sin_port = htons(port);
	client->addr.sin_addr.s_addr = inet_addr(ip);
	
	///connect() client
	if(connect(client->sockfd, (struct sockaddr*)&(client->addr), sizeof(struct sockaddr)) < 0){
		printf("	| ! Client : In client_init()... failed to connect()\n");
		return NULL;
	}
	
	///set client fd nonblock
	if(client_set_fd_nonblock(client->sockfd) < 0){
		printf("	| ! Client : In client_init()... failed to set client fd nonblock\n");
		return NULL;
	}	
	
	///create epoll for EPOLL_IN
	if( (client->epoll_in_fd =  epoll_create(MAX_EVENTS)) < 0){
		// *error = EPOLL_FAIL;
		printf("	| ! Client : In client_init()... failed to create epoll\n");
		return NULL;
	}

	printf("	| ! Client : In client_init()... epoll for epollin created  fd : %d\n",client_get_epoll_in_fd(client));
	
	///create epoll for EPOLL_OUT
	if( (client->epoll_out_fd = epoll_create(MAX_EVENTS)) < 0){
		// *error = EPOLL_FAIL;
		printf("	| ! Client : In client_init()... failed to create epoll\n");
		return NULL;
	}
	printf("	| ! Client : In client_init()... epoll for epollout created  fd : %d\n",client_get_epoll_out_fd(client));

	///set epoll event type
	struct epoll_event client_in_event;
	client_in_event.events = EPOLLIN;
	client_in_event.data.fd = client->sockfd;

	///add client fd to epoll
	if( (epoll_ctl(client->epoll_in_fd, EPOLL_CTL_ADD, client->sockfd, &client_in_event)) < 0){
		//*error = EPOLL_FAIL
	}

	///set epoll event type
	struct epoll_event client_out_event;
	client_out_event.events = EPOLLOUT;
	client_out_event.data.fd = client->sockfd;

	///add client fd to epoll
	if( (epoll_ctl(client->epoll_out_fd, EPOLL_CTL_ADD, client->sockfd, &client_out_event)) < 0){
		//*error = EPOLL_FAIL
		close(client->sockfd);
		return NULL;
	}


	printf("	| ! Client : client_init() done...\n");

	///return client_t* client
	return client;
	
}
int main(int argc, char* argv[]){
	client_t* client = client_init(argc, argv);
	client_connect(client);

	return 0;


}
