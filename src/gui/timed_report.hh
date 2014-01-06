#ifndef __TIMED_REPORT_HH
#define __TIMED_REPORT_HH
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

class TimedReport {
 public:
	TimedReport(unsigned int period);
	virtual void SendReport() = 0;
	virtual void Followup() = 0;
	virtual void SendOnRequest() = 0;
 protected:
	static void *TimedThread(void *arg);
	pthread_t _ThreadID;
	unsigned int _Period;

};
#endif
