#ifndef __PROCESS_REPORT_HH
#define __PROCESS_REPORT_HH

#include "report.hh"
#include "timed_report.hh"

typedef enum { DISK2CACHE = 0, CACHE2FILE, FILE2SERVER, SERVER2CLIENT } Stage;

class ProcessReport:public TimedReport {
 public:
	ProcessReport();
	void Update(Stage stage, bool work);
	void SendReport();
	void Followup();
	void SendOnRequest();
 private:
	void SendOneStage(Stage stage);
	bool status[4];
	bool worked[4];
	bool working[4];
};
#endif
