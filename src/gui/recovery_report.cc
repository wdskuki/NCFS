#include "recovery_report.hh"
#include "../filesystem/filesystem_common.hh"

extern ReportLayer *reportLayer;
extern struct ncfs_state *NCFS_DATA;

/*
void ProcessItem::Report(Stage stage,bool working){
	char* temp = (char*)malloc(3);
	temp[0] = PROCESS;
	temp[1] = stage;
	if(!working)
		temp[2] = 0;
	else
		temp[2] = 1;
	QueneItem __item(ALL,3,temp);	
	//fprintf(stderr,"\n\n\nhello\n\n\n");
	reportLayer->AddItem(__item);
	free(temp);
}*/

void RecoveryReport::Start(char *newdevice)
{
	char *temp = (char *)malloc(2);
	temp[0] = RECOVERY;
	temp[1] = 0;
	for (int i = 0; i < NCFS_DATA->disk_total_num; ++i) {
		if (NCFS_DATA->disk_status[i] == 1)
			continue;
		reportLayer->SendPath(ALL, temp, 2, NCFS_DATA->dev_name[i]);
	}
	reportLayer->SendPath(ALL, temp, 2, newdevice);
	reportLayer->SendPath(ALL, temp, 2, "/");
	free(temp);
}

void RecoveryReport::Finish(void)
{
	char *temp = (char *)malloc(2);
	temp[0] = RECOVERY;
	temp[1] = 1;
	QueneItem __item(ALL, 2, temp);
	reportLayer->AddItem(__item);
	free(temp);
}
