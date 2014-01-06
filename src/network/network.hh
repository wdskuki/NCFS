#ifndef __NETWORK_HH
#define __NETWORK_HH

#include <list>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;
#include "connection.hh"
#include "../utility/recovery.hh"
#include "../filesystem/filesystem_utils.hh"

#define	MAXCLIENT	1024
#define ALL			-1
class QueneItem {
 public:
	QueneItem(const QueneItem & item);
	 QueneItem(int _target, int _totallen, void *_payload);
	~QueneItem();
	int target;
	int totallen;
	void *payload;
};

class NetworkLayer {
 public:
	NetworkLayer(void);
	int SendItem(QueneItem item);
	virtual void CheckAllClients(fd_set * socks) = 0;
 protected:
	static void *ServerThread(void *arg);
	struct thread_arg {
		NetworkLayer *This;
		 thread_arg(NetworkLayer * t)
		:This(t) {
	}};
	void AcceptNewClient();
	pthread_t _ThreadID;
	int _ServerSocket;
	struct ClientRecordStruct {
		bool Connected;
		struct sockaddr_in Address;
		socklen_t AddressLen;
	} ClientRecord[MAXCLIENT];
	int _Maxfd;
};

void CheckClients(NetworkLayer * Layer, fd_set * socks);
#endif
