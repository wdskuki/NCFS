#include "folder_report.hh"
#include "../filesystem/filesystem_utils.hh"
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

#define ReadPath(x,y)	reportLayer->ReadPath(x,y)
#define SendPath(w,x,y,z)	reportLayer->SendPath(w,x,y,z)
extern ReportLayer *reportLayer;
extern FileSystemLayer *fileSystemLayer;

FILE *fpinprocess = NULL;
long long remaining = -1;

void ReadFolder(int sd)
{
	char path[PATH_MAX] = { 0 };
	ReadPath(path, sd);
	char fpath[PATH_MAX] = { 0 };
	char header[2] = { FOLDER, 0 };
	fileSystemLayer->ncfs_fullpath(fpath, path);
	fprintf(stderr, "Read Folder %s|\n", fpath);
	DIR *dp = opendir(fpath);
	if (dp == NULL)
		return;
	struct dirent *de;
	de = readdir(dp);
	do {
		if (strcmp(".", de->d_name) == 0)
			continue;
		if (strcmp("..", de->d_name) == 0)
			continue;
		struct stat st_buf;
		char tmppath[PATH_MAX] = { 0 };
		strcpy(tmppath, path);
		strcat(tmppath, de->d_name);
		fileSystemLayer->ncfs_fullpath(fpath, tmppath);
		stat(fpath, &st_buf);
		memset(tmppath, 0, PATH_MAX);
		strcpy(tmppath, de->d_name);
		if (S_ISDIR(st_buf.st_mode)) {
			strcat(tmppath, "/");
		}
		SendPath(sd, header, 2, tmppath);
	} while ((de = readdir(dp)) != NULL);
	closedir(dp);
	SendPath(sd, header, 2, "/");
}

void RecvFile(int sd)
{
	if (remaining < 0) {
		char path[PATH_MAX] = { 0 };
		ReadPath(path, sd);
		//      char* path = ReadPath(sd);
		long long remaining_n;
		recvn(sd, &remaining_n, 8);
		fprintf(stderr, "%s|\n", path);
		//fprintf(stderr,"%llX\n",remaining_n);
		//fprintf(stderr,"network order %lld\n",remaining_n);
		remaining = ntohll(remaining_n);
		fprintf(stderr, "file size = %lld\n", remaining);
		char fpath[PATH_MAX] = { 0 };
		fileSystemLayer->ncfs_mountpath(fpath, path);
		fpinprocess = fopen(fpath, "w");
		return;
	}
	int readsize;
	if (remaining >= 1024)
		readsize = 1024;
	else
		readsize = remaining;
	char buf[readsize];
	recvn(sd, buf, readsize);
	fwrite(buf, 1, readsize, fpinprocess);
	remaining -= readsize;
	//fprintf(stderr,"read %d, remain %lld\n",readsize,remaining);
	if (remaining == 0) {
		fclose(fpinprocess);
		fpinprocess = NULL;
		remaining = -1;
	}
}

void SendFile(int sd)
{
	//char *path = ReadPath(sd);
	char path[PATH_MAX] = { 0 };
	ReadPath(path, sd);
	char fpath[PATH_MAX] = { 0 };
	fileSystemLayer->ncfs_mountpath(fpath, path);
	struct stat statbuf;
	stat(fpath, &statbuf);
	int filesize = htonl(statbuf.st_size);
	char content[6];
	content[0] = FOLDER;
	content[1] = 2;
	memcpy(&content[2], &filesize, 4);
	fprintf(stderr, "%s size %d\n", fpath, filesize);
	QueneItem __item(sd, 6, &content);
	reportLayer->AddItem(__item);
	FILE *fp = fopen(fpath, "r");
	char buf[1030] = { 0 };
	buf[0] = FOLDER;
	buf[1] = 2;

	int retstat;
	while ((retstat = fread(&buf[2], 1, 1024, fp)) > 0) {
		//memset(&buf[2],1,retstat);
		QueneItem __item(sd, retstat + 2, buf);
		reportLayer->AddItem(__item);
		memset(&buf[2], 0, 1024);
	}
	fclose(fp);
}

void DeleteFile(int sd)
{
	//char *path = ReadPath(sd);
	char path[PATH_MAX] = { 0 };
	ReadPath(path, sd);
	char fpath[PATH_MAX] = { 0 };
	fileSystemLayer->ncfs_mountpath(fpath, path);
	char command[PATH_MAX + 10] = { 0 };
	fprintf(stderr, "Delete %s|\n", fpath);
	sprintf(command, "rm -rf '%s'", fpath);
	system(command);
}

void Get1FileName(int sd, char fpath1[PATH_MAX])
{
	char path[PATH_MAX] = { 0 };
	ReadPath(path, sd);
	memset(fpath1, 0, PATH_MAX);
	fileSystemLayer->ncfs_mountpath(fpath1, path);
}

void Get2FileNames(int sd, char fpath1[PATH_MAX], char fpath2[PATH_MAX])
{
	Get1FileName(sd, fpath1);
	Get1FileName(sd, fpath2);
}

void CopyFile(int sd)
{
	char path[PATH_MAX] = { 0 };
	ReadPath(path, sd);
	char fpath1[PATH_MAX] = { 0 };
	fileSystemLayer->ncfs_mountpath(fpath1, path);
	memset(path, 0, PATH_MAX);
	ReadPath(path, sd);
	char fpath2[PATH_MAX] = { 0 };
	fileSystemLayer->ncfs_mountpath(fpath2, path);
	char command[PATH_MAX * 2 + 10] = { 0 };
	fprintf(stderr, "Copy %s %s|\n", fpath1, fpath2);
	sprintf(command, "cp -rf '%s' '%s'", fpath1, fpath2);
	system(command);
}

void MoveFile(int sd)
{
	char fpath1[PATH_MAX] = { 0 };
	char fpath2[PATH_MAX] = { 0 };
	Get2FileNames(sd, fpath1, fpath2);
	char command[PATH_MAX * 2 + 10] = { 0 };
	fprintf(stderr, "Move %s %s|\n", fpath1, fpath2);
	sprintf(command, "mv -f '%s' '%s'", fpath1, fpath2);
	system(command);
}

void NewFolder(int sd)
{
	char fpath[PATH_MAX] = { 0 };
	Get1FileName(sd, fpath);
	char command[PATH_MAX + 10] = { 0 };
	fprintf(stderr, "New Folder %s|\n", fpath);
	sprintf(command, "mkdir '%s'", fpath);
	system(command);
}

void FolderReader::ProcessRequest(int sd)
{
	char inbyte;
	recvn(sd, &inbyte, 1);
	switch (inbyte) {
	case 0:
		ReadFolder(sd);
		break;
	case 1:
		RecvFile(sd);
		break;
	case 2:
		SendFile(sd);
		break;
	case 3:
		CopyFile(sd);
		break;
	case 4:
		DeleteFile(sd);
		break;
	case 5:
		MoveFile(sd);
		break;
	case 6:
		NewFolder(sd);
		break;
	}
}
