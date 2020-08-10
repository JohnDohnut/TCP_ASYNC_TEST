#ifndef __CLIENT_H__
#define __CLIENT_H__
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

typedef struct client_s client_t;

struct client_s{
	int sockfd;
	struct sockaddr_in addr;
	int epoll_in_fd;
	int epoll_out_fd;
	struct epoll_event in_events[MAX_EVENTS];
	struct epoll_event out_events[MAX_EVENTS];
};

client_t* client_init();
int client_connect();
int client_destroy();

#endif
