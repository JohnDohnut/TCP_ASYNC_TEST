#ifndef __SERVER_H__
#define __SERVER_H__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <netdb.h>
#include <unistd.h>

#include "../error/error.h"
#include "../protocol/protocol.h"

#define MAX_MAIN_ARG_LEN 128
#define MAX_EVENTS 1024 /// maximum number of events which can be dealt on single epoll_wait() call.
#define TIMEOUT 8

typedef struct server_transact_s server_trans_t;
struct server_transact_s{
	int recv_bytes;
	int send_bytes;
	int is_head_recv;
	int is_body_recv;
	int is_head_sent;
	int is_body_sent;
};

typedef struct server_s server_t;
struct server_s{
	int sockfd; ///server fd
	struct sockaddr_in addr; /// server addr
	int epoll_fd; /// fd of epoll
	struct epoll_event events [MAX_EVENTS]; /// array to store event structures
	
};



server_t* server_init(int argc, char** argv);
int server_connect(server_t*);
int server_destroy(server_t*);
#endif


