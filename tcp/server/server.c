#include "server.h"

///init trans_t*

server_trans_t* server_trans_init(){
	server_trans_t* trans = malloc(sizeof(server_trans_t));
	memset(trans,0,sizeof(server_trans_t));
	return trans;
}

///getters for server_t

int server_get_sockfd(server_t* server){
	if(server == NULL){
		return -1;
	}
	return server->sockfd;
}
int server_check_fd (int fd){
	int error = 0;
	socklen_t err_len = sizeof( error);
	if( getsockopt( fd, SOL_SOCKET, SO_ERROR, &error, &err_len) < 0){
		return FD_ERR;
	}
	return NO_ERR;	
}
struct sockaddr_in* server_get_addr(server_t* server){
	if(server==NULL){
		return NULL;
	}
	return &(server->addr);
}
int server_get_epoll_fd(server_t* server){
	if(server == NULL){
		return -1;
	}
	return server->epoll_fd;
}
struct epoll_event* server_get_events(server_t* server){
	if(server == NULL){
		return NULL;	
	}
	return server->events;
}

int server_set_fd_nonblock(int fd){
	int curr_flag = 0;
	/// get current flag set on fd.
	if( (curr_flag = fcntl(fd, F_GETFL, 0)) < 0){
		printf("	| ! Server : fcntl error on get flag\n");
		return FD_ERR;
	}
	/// set fd flag rv(current flag)|O_NONBLOCK
	if( fcntl(fd, F_SETFL, curr_flag | O_NONBLOCK) < 0){
		printf("	| ! Server : fcntl error on set flag\n");
		return FD_ERR;
	}
	return NO_ERR;
}



int server_process_data(int fd, server_trans_t* trans, protocol_packet_t* recv_packet, protocol_packet_t* send_packet){
	
	int write_rv = NO_ERR;
	int read_rv = NO_ERR;

	if((read_rv = read(fd, recv_packet+trans->recv_bytes, sizeof(protocol_packet_t)-trans->recv_bytes)) < 0){
		if(errno == EAGAIN || errno == EWOULDBLOCK){

			return ERRNO_EAGAIN;
		}
		else if(errno == EINTR){
			printf("	| ! Server : In server_process_data()... INTERRUPTed while read()\n");
			return INTERRUPT;
		}
		else {
			printf("	| ! Server : In server_process_data()... NEGATIVE_BYTE while read()\n");
			return NEGATIVE_BYTE;
		}
	}
	else if(read_rv == 0){
		printf("	| ! Server : In server_process_data()... zero byte while read()\n");
		return ZERO_BYTE;
	}
	else{
		trans->recv_bytes += read_rv;
		
	}

	/// if all recieved.
	if(trans->recv_bytes == sizeof(protocol_packet_t)){
		memcpy(send_packet, recv_packet, sizeof(protocol_packet_t));
		printf("	| @ Server : msg recieved.\n");
		printf("	| @ Server : msg type : %d\n",recv_packet->header.packet_type);
		printf("		     seq_id : %d \n",recv_packet ->header.seq_id);
		printf("	| @ Server : body : %s \n",recv_packet->body);
		if( (write_rv = write(fd, recv_packet+trans->send_bytes, sizeof(protocol_packet_t)-trans->send_bytes)) < 0){
			if(errno == EAGAIN || errno == EWOULDBLOCK){
				return ERRNO_EAGAIN;
			}
			else if(errno == EINTR){
				printf("	| ! Server : In server_process_data()... INTERRUPTed while write()\n");
				return INTERRUPT;
			}
			else {
				printf("	| ! Server : In server_process_data()... NEGATIVE_BYTE while write()\n");
				return NEGATIVE_BYTE;
			}
		}
		else if (write_rv == 0){
			printf("	| ! Server : In server_process_data()... zero byte while write()\n");
			return ZERO_BYTE;			
		}
		else{
			trans->send_bytes += write_rv;
		}
	}
	memset(trans,0,sizeof(server_trans_t));
	return NO_ERR;
	
}

server_t* server_init(int argc, char** argv){
	/// error check
/*	if(error != NO_ERR){
		printf("	| ! Server : There was an error during previous process\n");
		return NULL;
	}*/
	/// main param check
	if(argc!=3){
		// *error = WRONG_MAIN_PARAM;
		printf("	| ! Server : In server_init()... wrong main parameter\n");
		return NULL;
	}
	/// server_t* malloc	
	server_t* server = malloc(sizeof(server_t));
	if(server == NULL){
		// *error = SERVER_NULL;
		printf("	| ! Server : In server_init()... malloc() failed\n");
		return NULL;
	}
	/// initialze server_t*
	if(memset(server,0,sizeof(server_t)) == NULL){
		// *error = UNKNOWN;
		printf("	| ! Server : In server_init()... memset() server_t failed\n");
		return NULL;
	}
	/// declare and initialize space for server addr
	char ip[MAX_MAIN_ARG_LEN];
	int port = -1;

	if(memset(ip,0,sizeof(ip)) == NULL){
		// *error = UNKNOWN;
		printf("	| ! Server : In server_init()... memset() ip buf failed\n");
	}
	strncpy(ip,argv[1],MAX_MAIN_ARG_LEN);
	port = atoi(argv[2]);

	/// socket() server
	if ( (server->sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		// *error = SOCK_FAIL;
		printf("	| ! Server : In server_init()... socket() failed\n");
		return NULL;
	}	
	server->addr.sin_family = AF_INET;
	server->addr.sin_port = htons(port);
	server->addr.sin_addr.s_addr = INADDR_ANY;
	
	/// bind() server
	
	if( bind(server->sockfd, (struct sockaddr *)&(server->addr), sizeof(struct sockaddr)) == -1){
		// *error = BIND_FAIL;
		printf("	| ! Server : In server_init()... bind() failed\n");
		return NULL;
	}
	///server listen() 
	if( (listen(server->sockfd, 5)) < 0){
		printf("	 | ! Server : In server_init()... failed to listen()\n");
		return NULL;
	}

	///create epoll
	if( (server->epoll_fd =  epoll_create(MAX_EVENTS)) < 0){
		// *error = EPOLL_FAIL;
		printf("	| ! Server : In server_init()... failed to create epoll\n");
		return NULL;
	}

	printf("	| ! Server : In server_init()... Epoll created fd : %d\n",server_get_epoll_fd(server));

	///set epoll event type
	struct epoll_event server_event;
	server_event.events = EPOLLIN;
	server_event.data.fd = server->sockfd;

	///add server fd to epoll
	if( (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, server->sockfd, &server_event)) < 0){
		//*error = EPOLL_FAIL
	}

	printf("	| ! Server : server_init() done...\n");
	
	///return server_t* server
	return server;
	
}
int server_connect(server_t* server){
	if(server == NULL){
		printf("	| ! Server : In server_connect()... server is NULL\n");
		return SERVER_NULL;
	}
	int rv =0;
	int event_count;
	protocol_packet_t* recv_packet = malloc(sizeof(protocol_packet_t));
	protocol_packet_t* send_packet = malloc(sizeof(protocol_packet_t));
	memset(recv_packet ,0, sizeof(protocol_packet_t));
	memset(send_packet ,0, sizeof(protocol_packet_t));

	server_trans_t* trans = server_trans_init();
	///accept client
	int client_fd = -1; 
	struct sockaddr_in client_addr;
	int client_addr_len = sizeof(struct sockaddr_in);
	memset(&client_addr, 0, client_addr_len);
	if( (client_fd = accept(server->sockfd, (struct sockaddr*)(&client_addr), (socklen_t*)(&client_addr_len))) < 0){
		printf("	| ! Server : In server_connect()... failed to accpet()\n");
		return ACCEPT_FAIL;
	}
	
	/// set client fd nonblock
	if(server_set_fd_nonblock(client_fd) < 0){
		return FD_ERR;
	}
	
	/// add client fd to epoll
	struct epoll_event client_event;
	client_event.events = EPOLLIN;
	client_event.data.fd = client_fd;
	epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, client_fd, &client_event);
	/// prepare vars

	int i=0;
	///temp 
		char buff[1024];
		int rcv_bytes;
		int flag=0;;
	/// handle client 
	while(1) {
		if(flag == 1){
			break;
		}
		/// check server_fd, client_fd
		if(server_check_fd(server->sockfd)|| server_check_fd(client_fd)){
			printf("	| ! Server : In server_connect()... server/client fd error\n");
			if(server_check_fd(server->sockfd) != NO_ERR){
				printf("	| ! Server : In server_connect()... server terminated\n");
				return SERVER_NULL;
			}
			return FD_ERR;
		}
		/// get number of event occurred.
		event_count = epoll_wait(server->epoll_fd, server->events, MAX_EVENTS, TIMEOUT);
		/// check fd
		if(event_count < 0){
			///error
			printf("	| ! Server : In server_connect()... event_count < 0\n");
			return EPOLL_ERR;
		}
		else if (event_count == 0){
			continue;
		}
		else if (event_count >0){
			
			for(i=0;i<event_count;i++){
				if(server->events[i].data.fd == server->sockfd){

				}	
				else if( server->events[i].data.fd == client_fd){
					if(server_process_data(client_fd,trans,recv_packet,send_packet)<NO_ERR){
						flag = 1;
						close(client_fd);
						break;
						
					}
				}	
	
	
			}
			memset(buff,0,1024);
		}

	}	
	return NO_ERR;
	 


}
int server_destroy(server_t* server){
	close(server_get_epoll_fd(server));
	close(server->sockfd);
	free(server);
	return 0;
}





int main(int argc, char* argv[]){
	server_t* server = server_init(argc, argv);
	if(server == NULL){
		printf("	| ! Server : server_init failed\n");
		return UNKNOWN;
	}
	server_connect(server);
	server_destroy(server);
	return 0;
}




