#ifndef __mysftp_h__
#define __mysftp_h__

#include <stdlib.h>		//exit(), malloc()
#include <stdio.h>		//printf()
#include <string.h>		//strcmp(), memset(), memcpy()
#include <sys/socket.h>		//socket(), setsockopt(), bind(), connect(), listen()
#include <sys/types.h>		//setsockopt(), bind(), connect(), stat(), open()
#include <netdb.h>		//gethostbyname()
#include <unistd.h>		//close(), stat()
#include <errno.h>
#include <sys/stat.h>		//stat(), open()
#include <fcntl.h>		//open()
#include <limits.h>		//INT_MAX
#include <netinet/in.h>		//inet_aton()
#include <arpa/inet.h>		//inet_aton()

int build_server(unsigned short int port, struct sockaddr_in *servaddr);
int build_client(char *ip, unsigned short int port,
		 struct sockaddr_in *servaddr);
int sendn(int sd, const void *buf, int buf_len);
int recvn(int sd, void *buf, int buf_len);
uint64_t htonll(uint64_t host_longlong);
uint64_t ntohll(uint64_t host_longlong);
#endif
