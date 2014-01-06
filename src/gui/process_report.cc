#include "process_report.hh"

extern ReportLayer *reportLayer;

 ProcessReport::ProcessReport():TimedReport(250)
{
	for (int i = 0; i < 4; ++i) {
		status[i] = false;
		worked[i] = false;
		working[i] = false;
	}
}

void ProcessReport::Update(Stage stage, bool work)
{
	if (work) {
		working[stage] = true;
	}
	status[stage] = work;
}

void ProcessReport::SendReport()
{
	for (int i = DISK2CACHE; i <= SERVER2CLIENT; ++i) {
		if (worked[i] != working[i]) {
			//fprintf(stderr,"Sent %d %d\n",i,working[i]);
			SendOneStage((Stage) i);
		}
		worked[i] = working[i];
		working[i] = status[i];
	}
}

void ProcessReport::SendOnRequest()
{
	SendReport();
}

void ProcessReport::Followup()
{
}

void ProcessReport::SendOneStage(Stage stage)
{
	char *temp = (char *)malloc(3);
	temp[0] = PROCESS;
	temp[1] = stage;
	if (!working[stage])
		temp[2] = 0;
	else
		temp[2] = 1;
	QueneItem __item(ALL, 3, temp);
	//fprintf(stderr,"\n\n\nhello\n\n\n");
	reportLayer->AddItem(__item);
	free(temp);
}
