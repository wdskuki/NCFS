#include "storage.hh"
#include "../filesystem/filesystem_common.hh"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../gui/process_report.hh"
#include "../gui/diskusage_report.hh"
#include "../filesystem/filesystem_utils.hh"

extern struct ncfs_state *NCFS_DATA;
extern FileSystemLayer *fileSystemLayer;
extern ProcessReport *processReport;

/*************************************************************************
 * Private functions
 *************************************************************************/
/*       
 * search_disk_id: Search for disk id provided by device path
 *
 * @param path: device path
 *
 * @return: 
 */
int StorageLayer::search_disk_id(const char *path)
{
	int i;
	for (i = 0; i < NCFS_DATA->disk_total_num; ++i) {
		if (strcmp(NCFS_DATA->dev_name[i], path) == 0)
			return i;
	}
	return -1;
}

/*************************************************************************
 * Public functions
 *************************************************************************/
/*
 * Constructor
 */
StorageLayer::StorageLayer()
{
}

/*
 * DiskRead: Read data from disk
 * 
 * @param path: file path
 * @param buf: data buffer
 * @param size: size
 * @param offset: offset
 * @return: size of successful read
 */
int StorageLayer::DiskRead(const char *path, char *buf, long long size,
			   long long offset)
{
	if (NCFS_DATA->no_gui == 0) {
		processReport->Update(DISK2CACHE, true);
	}
	int fd = open(path, O_RDONLY);

	if (fd < 0) {
		int id = search_disk_id(path);
		printf("///DiskRead %s failed\n", path);
		fileSystemLayer->set_device_status(id, 1);
		if (NCFS_DATA->no_gui == 0) {
			processReport->Update(DISK2CACHE, false);
		}
		return fd;
	}
	int retstat = pread(fd, buf, size, offset);
	if (retstat <= 0) {
		int id = search_disk_id(path);
		fileSystemLayer->set_device_status(id, 1);
		printf("///DiskRead %s failed\n", path);
		close(fd);
		if (NCFS_DATA->no_gui == 0) {
			processReport->Update(DISK2CACHE, false);
		}
		return retstat;
	}
	close(fd);
	if (NCFS_DATA->no_gui == 0) {
		processReport->Update(DISK2CACHE, false);
	}
	return retstat;
}

/*
 * DiskWrite: Write data from disk
 * 
 * @param path: file path
 * @param buf: data buffer
 * @param size: size
 * @param offset: offset
 * @return: size of successful write
 */
int StorageLayer::DiskWrite(const char *path, const char *buf, long long size,
			    long long offset)
{
	if (NCFS_DATA->no_gui == 0) {
		processReport->Update(DISK2CACHE, true);
	}
	int fd = open(path, O_WRONLY);

	if (fd < 0) {
		int id = search_disk_id(path);
		fileSystemLayer->set_device_status(id, 1);
		printf("///DiskWrite %s failed\n", path);
		if (NCFS_DATA->no_gui == 0) {
			processReport->Update(DISK2CACHE, false);
		}
		return fd;
	}
	int retstat = pwrite(fd, buf, size, offset);
	if (retstat <= 0) {
		int id = search_disk_id(path);
		fileSystemLayer->set_device_status(id, 1);
		printf("///DiskWrite %s failed\n", path);
		close(fd);
		if (NCFS_DATA->no_gui == 0) {
			processReport->Update(DISK2CACHE, false);
		}
		return retstat;
	}
	close(fd);
	if (NCFS_DATA->no_gui == 0) {
		processReport->Update(DISK2CACHE, false);
	}
	return retstat;
}
