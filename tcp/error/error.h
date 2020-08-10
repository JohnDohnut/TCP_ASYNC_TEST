#ifndef	__ERROR_H__
#define __ERROR_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

typedef enum error error_t;
enum error{
	ERRNO_EAGAIN = 1,
	NO_ERR = 0,
	SOCK_FAIL = -1,
	BIND_FAIL = -2,
	SERVER_NULL = -3,
	CLIENT_NULL = -3,
	ARG_NULL = -3,
	LISTEN_FAIL = -4,
	ACCEPT_FAIL = -4,
	FD_ERR = -5,
	EPOLL_ERR = -6,
	INTERRUPT = -7,
	NEGATIVE_BYTE = -8,
	ZERO_BYTE = -9,
	PROC_DATA_ERR = -11,

	WRONG_MAIN_PARAM = -99,
	UNKNOWN = -999
	

};

#endif
