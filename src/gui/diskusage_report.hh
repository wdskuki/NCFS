#ifndef __DISKUSAGE_REPORT_HH
#define __DISKUSAGE_REPORT_HH
#include "report.hh"
#include "timed_report.hh"

class DiskusageReport:public TimedReport {
 public:
	DiskusageReport(unsigned int numofblock);
	void Update(int diskid, int blockno, char status);
	void ReportStatus(int diskid, int status);
	void SendReport();
	void Followup();
	void SendOnRequest();
	void ProcessRequest(int id);
	//void ReportStatus(int diskid, int status);
 private:
	void SendOneDisk(int target, int diskid);
	char **diskusage;
	bool *touched;
	unsigned int _numofblock;
	unsigned int _scale;
};
#endif
