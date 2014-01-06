#include "network.hh"

extern FileSystemLayer *fileSystemLayer;
extern bool terminated;

QueneItem::QueneItem(const QueneItem & item)
{
	target = item.target;
	totallen = item.totallen;
	payload = calloc(totallen, 1);
	memcpy(payload, item.payload, totallen);
}

QueneItem::QueneItem(int _target, int _totallen, void *_payload)
{
	target = _target;
	totallen = _totallen;
	//payload = _payload;
	payload = calloc(totallen, 1);
	memcpy(payload, _payload, totallen);
}

QueneItem::~QueneItem()
{
	free(payload);
}

NetworkLayer::NetworkLayer(void)
{
	for (int i = 0; i < MAXCLIENT; ++i) {
		ClientRecord[i].Connected = false;
	}
	struct sockaddr_in ServerAddress;
	_Maxfd = 0;
	_ServerSocket = build_server(1234, &ServerAddress);
	fprintf(stderr, "Build Server, SocketFS = %d\n", _ServerSocket);
	pthread_create(&_ThreadID, NULL, &NetworkLayer::ServerThread,
		       new thread_arg(this));
}

int NetworkLayer::SendItem(QueneItem item)
{
	int __count = 0;
	int __retstat;
	if (item.target == ALL) {
		for (int i = 0; i <= _Maxfd; ++i) {
			__retstat = 0;
			if (ClientRecord[i].Connected)
				__retstat =
				    sendn(i, item.payload, item.totallen);
			else
				continue;
			if (__retstat < 0)
				return 0;
			else
				++__count;
		}

	} else {
		__retstat = 0;
		if (ClientRecord[item.target].Connected)
			__retstat =
			    sendn(item.target, item.payload, item.totallen);
		if (__retstat < 0)
			return 0;
		else
			++__count;
	}
	return __count;
}

void NetworkLayer::AcceptNewClient()
{
	struct sockaddr_in __cliaddress;
	socklen_t __clilen = sizeof(struct sockaddr_in);
	int __clientfd =
	    accept(_ServerSocket, (struct sockaddr *)&__cliaddress, &__clilen);
	if (__clientfd > MAXCLIENT) {
		close(__clientfd);
		fprintf(stderr, "Server is full of Clients\n");
		return;
	}
	ClientRecord[__clientfd].Connected = true;
	ClientRecord[__clientfd].Address = __cliaddress;
	ClientRecord[__clientfd].AddressLen = __clilen;
	if (__clientfd > _Maxfd)
		_Maxfd = __clientfd;
	fprintf(stderr, "Client Connected, %s : %d\n",
		inet_ntoa(__cliaddress.sin_addr), ntohs(__cliaddress.sin_port));
	return;
}

void CheckClients(NetworkLayer * This, fd_set * socks)
{
	This->CheckAllClients(socks);
}

void *NetworkLayer::ServerThread(void *arg)
{
	thread_arg *targ = static_cast < thread_arg * >(arg);
	NetworkLayer *This = targ->This;
	fd_set socks;
	fd_set sdsocks;
	while (1) {
		FD_ZERO(&socks);
		FD_ZERO(&sdsocks);
		FD_SET(0, &socks);
		FD_SET(This->_ServerSocket, &socks);
		int __Maxfd = This->_Maxfd;
		This->_Maxfd = This->_ServerSocket;
		for (int i = 0; i <= __Maxfd; ++i) {
			if (This->ClientRecord[i].Connected == true) {
				FD_SET(i, &socks);
				FD_SET(i, &sdsocks);
				if (i > This->_Maxfd)
					This->_Maxfd = i;
			}
		}
		//int readsocks = select(This->_Maxfd + 1, &socks, &sdsocks, NULL,NULL);
		int readsocks =
		    select(This->_Maxfd + 1, &socks, NULL, NULL, NULL);
		if (readsocks < 0) {
			perror("select");
			exit(EXIT_FAILURE);
		} else if (readsocks == 0) {

		} else {
			if (FD_ISSET(This->_ServerSocket, &socks))
				This->AcceptNewClient();
			else if (FD_ISSET(0, &socks)) {
				fileSystemLayer->process_command();
			}
			CheckClients(This, &socks);
		}
		if (terminated)
			break;
	}

	return NULL;
}
