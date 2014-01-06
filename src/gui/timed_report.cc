#include "timed_report.hh"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

void *TimedReport::TimedThread(void *arg)
{
	TimedReport *This = static_cast < TimedReport * >(arg);
	while (1) {
		This->SendReport();
		This->Followup();
		usleep(This->_Period * 1000);
	}
	return NULL;
}

TimedReport::TimedReport(unsigned int period)
{
	_Period = period;
	pthread_create(&_ThreadID, NULL, TimedThread, (void *)this);
}
