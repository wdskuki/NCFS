#include "diskusage_report.hh"

extern ReportLayer *reportLayer;
extern struct ncfs_state *NCFS_DATA;

DiskusageReport::DiskusageReport(unsigned int numofblock):TimedReport(1000)
{
	diskusage = new char *[NCFS_DATA->disk_total_num];
	touched = new bool[NCFS_DATA->disk_total_num];
	for (int i = 0; i < NCFS_DATA->disk_total_num; ++i) {
		diskusage[i] = new char[numofblock];
		touched[i] = true;
		int __blocksize = NCFS_DATA->chunk_size;
		unsigned int __usedblock =
		    (NCFS_DATA->free_offset[i] - 1) / __blocksize;
		if (((NCFS_DATA->free_offset[i] - 1) % __blocksize) > 0)
			__usedblock++;
		for (unsigned int j = 0; j < __usedblock; ++j)
			diskusage[i][j] = 1;
		for (unsigned int j = __usedblock; j < numofblock; ++j)
			diskusage[i][j] = 0;
	}
	_numofblock = numofblock;
	_scale =
	    (NCFS_DATA->free_offset[0] + NCFS_DATA->free_size[0]) / numofblock;
	//gettimeofday(&prev,NULL);
}

void DiskusageReport::Update(int diskid, int blockno, char status)
{
	if ((diskid < 0) || (diskid > NCFS_DATA->disk_total_num)
	    || (blockno < 0))
		return;
	unsigned __targetblock = blockno / _scale;
	if (diskusage[diskid][__targetblock] == status)
		return;
	diskusage[diskid][__targetblock] = status;
	touched[diskid] = true;
	return;
}

void DiskusageReport::ReportStatus(int diskid, int status)
{
	char *temp = (char *)malloc(7);
	temp[0] = DISKUSAGE;
	temp[1] = 1;
	int diskid_n = htonl(diskid);
	memcpy(&temp[2], &diskid_n, 4);
	if (status == 1)
		temp[6] = 1;
	else
		temp[6] = 0;
	QueneItem __item(ALL, 7, temp);
	reportLayer->AddItem(__item);
	free(temp);
}

void DiskusageReport::SendReport()
{
	for (int i = 0; i < NCFS_DATA->disk_total_num; ++i) {
		if (touched[i])
			SendOneDisk(ALL, i);
		touched[i] = false;
	}
}

void DiskusageReport::SendOnRequest()
{
}

void DiskusageReport::ProcessRequest(int id)
{
	int diskid_n;
	int diskid;
	recvn(id, &diskid_n, 4);
	diskid = ntohl(diskid_n);
	SendOneDisk(id, diskid);
}

void DiskusageReport::Followup()
{
}

void DiskusageReport::SendOneDisk(int target, int diskid)
{
	char *content = (char *)calloc(114, 1);
	content[0] = DISKUSAGE;
	content[1] = 0;
	int diskid_n = htonl(diskid);
	memcpy(&content[2], &diskid_n, 4);
	int freesize_n =
	    htonl(NCFS_DATA->free_size[diskid] * NCFS_DATA->chunk_size);
	memcpy(&content[6], &freesize_n, 4);
	int freeoffset_n =
	    htonl(NCFS_DATA->free_offset[diskid] * NCFS_DATA->chunk_size);
	memcpy(&content[10], &freeoffset_n, 4);
	memcpy(&content[14], diskusage[diskid], 100);
	QueneItem __item(target, 114, content);
//      fprintf(stderr,"Disk Usage %d used %d\n",diskid,__usedblock);
//      for(int i =0; i < 105; ++i)
//              fprintf(stderr,"%c",content[i] + '0');
//      fprintf(stderr,"\n");
	reportLayer->AddItem(__item);
	free(content);
}
