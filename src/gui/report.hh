#ifndef __REPORT_HH
#define __REPORT_HH

#define PROCESS		0
#define DISKUSAGE	1
#define RECOVERY	2
#define FOLDER		3

#include "../network/network.hh"

class ReportLayer:public NetworkLayer {
 public:
	ReportLayer(void);
	void AddItem(QueneItem item);
	bool isEmpty(void);
	int SendAll(void);
	void CheckAllClients(fd_set * socks);
	void ReadPath(char path[PATH_MAX], int sd);
	void SendPath(int sd, char *header, int headersize, const char *path);
 protected:
 private:
	/*
	   static void* ServerThread(void* arg);
	   struct thread_arg{
	   ReportLayer* This;
	   thread_arg(ReportLayer* t)
	   : This(t){}
	   };
	 */
	void ModuleDispatcher(char Module, int id);
	 list < QueneItem > _QueneList;
	pthread_mutex_t _QueneMutex;
};
#endif
