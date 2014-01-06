#include "connection.hh"

int build_server(unsigned short int port, struct sockaddr_in *servaddr)
{
	int socket_id;
	int one = 1;

	memset(servaddr, 0, sizeof(struct sockaddr_in));
	servaddr->sin_family = AF_INET;
	servaddr->sin_port = htons(port);
	servaddr->sin_addr.s_addr = htonl(INADDR_ANY);

	if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket():");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))
	    == -1) {
		perror("setsockopt():");
		exit(EXIT_FAILURE);
	}

	if (bind
	    (socket_id, (struct sockaddr *)servaddr,
	     sizeof(struct sockaddr_in)) < 0) {
		perror("bind():");
		exit(EXIT_FAILURE);
	}

	if (listen(socket_id, 50) < 0) {
		perror("listen():");
		exit(EXIT_FAILURE);
	}
	return socket_id;
}

int build_client(char *ip, unsigned short int port,
		 struct sockaddr_in *servaddr)
{
	int socket_id;
	struct hostent *ht;
	int one = 1;

	memset(&ht, 0, sizeof(ht));
	memset(servaddr, 0, sizeof(struct sockaddr_in));
	servaddr->sin_family = AF_INET;
	servaddr->sin_port = htons(port);
	if ((ht = gethostbyname(ip)) == NULL) {
		perror("gethostbyname():");
		exit(EXIT_FAILURE);
	}
	memcpy(&(servaddr->sin_addr), ht->h_addr, ht->h_length);

	if ((socket_id = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket():");
		exit(EXIT_FAILURE);
	}

	if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one))
	    == -1) {
		perror("setsockopt():");
		exit(EXIT_FAILURE);
	}

	if (connect
	    (socket_id, (struct sockaddr *)servaddr,
	     sizeof(struct sockaddr)) < 0) {
		perror("connect():");
		exit(EXIT_FAILURE);
	}

	return socket_id;
}

int sendn(int sd, const void *buf, int buf_len)
{
	int n_left = buf_len;	// actual data bytes sent
	int n;
	while (n_left > 0) {
		if ((n =
		     send(sd, (char *)buf + (buf_len - n_left), n_left,
			  0)) < 0) {
			if (errno == EINTR)
				n = 0;
			else
				return -1;
		} else if (n == 0) {
			return 0;
		}
		n_left -= n;
	}
	return buf_len;
}

int recvn(int sd, void *buf, int buf_len)
{
	int n_left = buf_len;
	int n = 0;
	while (n_left > 0) {
		if ((n =
		     recv(sd, (char *)buf + (buf_len - n_left), n_left,
			  0)) < 0) {
			if (errno == EINTR)
				n = 0;
			else
				return -1;
		} else if (n == 0) {
			return 0;
		}
		n_left -= n;
	}
	return buf_len;
}

/**
  * byte ordering function for 64bit variable
  */
uint64_t htonll(uint64_t host_longlong)
{
	int x = 1;

	/* little endian */
	if (*(char *)&x == 1)
		return ((((uint64_t) htonl(host_longlong)) << 32) +
			htonl(host_longlong >> 32));

	/* big endian */
	else
		return host_longlong;
}

uint64_t ntohll(uint64_t host_longlong)
{
	int x = 1;

	/* little endian */
	if (*(char *)&x == 1)
		return ((((uint64_t) ntohl(host_longlong)) << 32) +
			ntohl(host_longlong >> 32));

	/* big endian */
	else
		return host_longlong;

}
