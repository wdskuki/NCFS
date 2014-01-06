#include "report.hh"
#include "folder_report.hh"
#include "diskusage_report.hh"

extern DiskusageReport *diskusageLayer;

ReportLayer::ReportLayer(void)
{
	pthread_mutex_init(&_QueneMutex, NULL);
	//pthread_create(&_ThreadID,NULL,&ReportLayer::ServerThread,new thread_arg(this));
	//this->NetworkLayer::NetworkLayer();
}

void ReportLayer::AddItem(QueneItem item)
{
	pthread_mutex_lock(&_QueneMutex);
	SendItem(item);
	//_QueneList.push_back(item);
	pthread_mutex_unlock(&_QueneMutex);
}

bool ReportLayer::isEmpty(void)
{
	pthread_mutex_lock(&_QueneMutex);
	bool __empty = _QueneList.empty();
	pthread_mutex_unlock(&_QueneMutex);
	return __empty;
}

int ReportLayer::SendAll(void)
{
	pthread_mutex_lock(&_QueneMutex);
	//list<QueneItem>::iterator i;
	int __count = 0;
	while (!_QueneList.empty()) {
		int __retstat = SendItem(_QueneList.front());
		//fprintf(stderr,"%s ret %d\n",__func__,__retstat);
		if (__retstat < 0)
			break;
		++__count;
		_QueneList.pop_front();
	}
	pthread_mutex_unlock(&_QueneMutex);
	return __count;
}

void ReportLayer::CheckAllClients(fd_set * socks)
{
	for (int i = _ServerSocket + 1; i <= _Maxfd; ++i) {
		if (FD_ISSET(i, socks)) {
			char Module;
			int retstat = recvn(i, &Module, 1);
			if (retstat < 0) {
				perror("recv");
				exit(EXIT_FAILURE);
			} else if (retstat == 0) {
				fprintf(stderr, "Client disconnected %s %d\n",
					inet_ntoa(ClientRecord[i].Address.
						  sin_addr),
					ntohs(ClientRecord[i].Address.
					      sin_port));
				if (i == _Maxfd) {
					do {
						--_Maxfd;
					} while ((_Maxfd > 2)
						 && !ClientRecord[_Maxfd].
						 Connected);
				}
				ClientRecord[i].Connected = false;
			} else {
				ModuleDispatcher(Module, i);
			}
		}
	}
}

void ReportLayer::ReadPath(char path[PATH_MAX], int sd)
{
	int remaining_n;
	int remaining;
	recvn(sd, &remaining_n, 4);
	remaining = ntohl(remaining_n);
//      fprintf(stderr,"Path len%d\n",remaining);

	recvn(sd, path, remaining);
}

void ReportLayer::SendPath(int sd, char *header, int headersize,
			   const char *path)
{
	char *content = (char *)calloc(headersize + PATH_MAX + 1, 1);
	//fprintf(stderr,"%s %d %d %d %d\n",__func__,sizeof(header),sizeof(path),strlen(header),strlen(path));
	//content[0] = 3;
	//content[1] = 0;
	//strcpy(&content[2],path);
	memcpy(content, header, headersize);
	memcpy(&content[headersize], path, strlen(path));
	content[strlen(path) + headersize] = '\\';
	QueneItem __item(sd, strlen(path) + headersize + 1, content);
	AddItem(__item);
	free(content);
}

void ReportLayer::ModuleDispatcher(char Module, int id)
{
	switch (Module) {
	case DISKUSAGE:
		diskusageLayer->ProcessRequest(id);
		break;
	case FOLDER:
		FolderReader::ProcessRequest(id);
		break;
	default:
		break;
	}
}
