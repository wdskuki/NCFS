#include "filesystem_common.hh"
#include "filesystem_utils.hh"
#include "../coding/coding.hh"
#include "../utility/recovery.hh"
#include "../gui/folder_report.hh"
#include "../config/config.hh"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include "../gui/diskusage_report.hh"

//NCFS context state
extern struct ncfs_state *NCFS_DATA;

//Add by zhuyunfeng on October 11, 2011 begin.
extern ConfigLayer* configLayer;
//Add by zhuyunfeng on October 11, 2011 end.

extern DiskusageReport *diskusageLayer;

enum param_type {
	illegal_param, disk_total_num, data_disk_num, chunk_size,
	    disk_raid_type, space_list_num,
	free_offset, free_size, dev_name, no_cache, no_gui, run_experiment
};

#define is_param(line, param_str)                                       \
	(strncmp((line), (param_str), sizeof(param_str) - 1) == 0)

#define global_param_int(ncfs_data, line, param, param_str)               \
	do {                                                                \
		if (is_param((line), (param_str))) {                            \
			(ncfs_data)->param = atoi((line) + sizeof(param_str));        \
			return param;                                               \
		}                                                               \
	} while(0)

#define disk_param_int(ncfs_data, line, param, param_str)                 \
	do {                                                                \
		char *dot = strchr(line, '.');                                  \
		if (dot && is_param(dot + 1, (param_str))) {                    \
			(ncfs_data)->param[atoi(line)] = atoi((dot + 1 + sizeof(param_str))); \
			return param;                                               \
		}                                                               \
	} while(0)

#define disk_param_string(ncfs_data, line, param, param_str)              \
	do {                                                                \
		char *dot = strchr(line, '.');                                  \
		if (dot && is_param(dot + 1, (param_str))) {                    \
			char *val = dot + 1 + sizeof(param_str);                    \
			size_t len = strlen(val);                                   \
			char *dev = (char *)calloc(len, sizeof(char));              \
			strncpy(dev, val, len - 1);                                 \
			(ncfs_data)->param[atoi((line))] = dev;                       \
			return param;                                               \
		}                                                               \
	} while(0)

/*************************************************************************
 * Private functions
 *************************************************************************/
/*
 * parse_setting_line: Parse a line in setting file
 *
 * @param ncfs_data: NCFS context state
 * @param line: a line in setting file
 *
 * @return: param_type
 */
int FileSystemLayer::parse_setting_line(struct ncfs_state *ncfs_data,
					char *line)
{
	global_param_int(ncfs_data, line, disk_total_num, "disk_total_num");
	global_param_int(ncfs_data, line, data_disk_num, "data_disk_num");
	global_param_int(ncfs_data, line, chunk_size, "chunk_size");
	global_param_int(ncfs_data, line, disk_raid_type, "disk_raid_type");
	//global_param_int(ncfs_data, line, space_list_num, "space_list_num");
	global_param_int(ncfs_data, line, no_cache, "no_cache");
	global_param_int(ncfs_data, line, no_gui, "no_gui");
	global_param_int(ncfs_data, line, run_experiment, "run_experiment");
	disk_param_int(ncfs_data, line, free_offset, "free_offset");
	disk_param_int(ncfs_data, line, free_size, "free_size");
	disk_param_string(ncfs_data, line, dev_name, "dev_name");
	return illegal_param;
}

/*************************************************************************
 * Public functions
 *************************************************************************/
/*
 * Constructor: Initialize File System Layer
 */
FileSystemLayer::FileSystemLayer()
{
	//codingStorageLayer = new CodingStorageLayer;
	//moved the above code to ncfs main, because it needs to read the raid_type.
}

//  All the paths I see are relative to the root of the mounted
//  filesystem.  In order to get to the underlying filesystem, I need to
//  have the mountpoint.  I'll save it away early on in main(), and then
//  whenever I need a path for something I'll call this to construct
//  it.
/*
 * ncfs_fullpath: get full path of a file
 *
 * @param fpath: output full path
 * @param path: file path
 */
void FileSystemLayer::ncfs_fullpath(char fpath[PATH_MAX], const char *path)
{
	strcpy(fpath, NCFS_DATA->rootdir);
	strncat(fpath, path, PATH_MAX);	// ridiculously long paths will
	return;
	// break here
}

void FileSystemLayer::ncfs_mountpath(char fpath[PATH_MAX], const char *path)
{
	strcpy(fpath, NCFS_DATA->mountdir);
	strncat(fpath, path, PATH_MAX);	// ridiculously long paths will
	return;

}

/*
 * get_disk_dev_name: Get a disk id's device name
 *
 * @param disk_id: disk ID
 *
 * @return: device name
 */
char *FileSystemLayer::get_disk_dev_name(int disk_id)
{
	return NCFS_DATA->dev_name[disk_id];
}

/*
 * round_to_block_size: Adjust size to around n block sizes
 *
 * @param size: size in bytes
 *
 * @return: rounded size in bytes
 */
int FileSystemLayer::round_to_block_size(int size)
{
	int result;
	int disk_block_size;

	disk_block_size = NCFS_DATA->chunk_size;

	if ((size % disk_block_size) != 0) {
		result = size + disk_block_size - (size % disk_block_size);
	} else {
		result = size;
	}

	return result;
}

/*
 * get_raid_setting: Get NCFS setting from setting file
 *
 * @param ncfs_data: NCFS context state
 */
void FileSystemLayer::get_raid_setting(struct ncfs_state *ncfs_data)
{
	FILE *fp = fopen("raid_setting", "r");
	char buf[80];
	while (fgets(buf, sizeof(buf), fp)) {
		if (parse_setting_line(ncfs_data, buf) == disk_total_num) {
			ncfs_data->disk_size =
			    (int *)malloc(sizeof(int) *
					  ncfs_data->disk_total_num);
			ncfs_data->free_offset =
			    (int *)malloc(sizeof(int) *
					  ncfs_data->disk_total_num);
			ncfs_data->free_size =
			    (int *)malloc(sizeof(int) *
					  ncfs_data->disk_total_num);
			ncfs_data->disk_status =
			    (int *)malloc(sizeof(int) *
					  ncfs_data->disk_total_num);
			ncfs_data->dev_name =
			    (char **)calloc(ncfs_data->disk_total_num,
					    sizeof(char *));
		}
	}
	fclose(fp);
}

/*
 * get_raid_metadata: Get NCFS metadata from metadata file
 *
 * @param ncfs_data: NCFS context state
 */
void FileSystemLayer::get_raid_metadata(struct ncfs_state *ncfs_data)
{
	int fd;
	int magic_no;
	int i;
	int offset;
	int disk_id;
	struct raid_metadata metadata;
	int retstat;
	int offset_space;
	int space_list_num;
	struct space_info temp_space_info;
	struct space_list *space_node;
	struct space_list *new_node;

	fd = open("raid_metadata", O_RDONLY);
	retstat = pread(fd, (char *)&magic_no, sizeof(int), 0);

	printf("***get_raid_metadata 0: magic_no = %d\n", magic_no);

	if (magic_no == MAGIC_NUMBER) {

		printf("***get_raid_metadata 1: magic_no = %d\n", magic_no);

		//get disk free size info
		for (i = 0; i < ncfs_data->disk_total_num; i++) {
			offset = sizeof(struct raid_metadata) * (i + 1);
			retstat =
			    pread(fd, (char *)&metadata,
				  sizeof(struct raid_metadata), offset);
			if (retstat > 0) {
				printf
				    ("***get_raid_metadata 3: offset = %d, disk_id=%d,free_offset=%d, free_size=%d\n",
				     offset, metadata.disk_id,
				     metadata.free_offset, metadata.free_size);

				disk_id = metadata.disk_id;
				ncfs_data->free_offset[disk_id] =
				    metadata.free_offset;
				ncfs_data->free_size[disk_id] =
				    metadata.free_size;
			}
		}

		//get disk space list info (deleted spaces)
		offset_space =
		    sizeof(struct raid_metadata) * (ncfs_data->disk_total_num +
						    1);
		retstat =
		    pread(fd, (char *)&space_list_num, sizeof(int),
			  offset_space);
		ncfs_data->space_list_num = space_list_num;
		printf("***get_raid_metadata 4: space_list_num = %d\n",
		       space_list_num);

		i = 0;
		space_node = ncfs_data->space_list_head;
		while ((i < space_list_num) && (retstat > 0)) {
			offset =
			    offset_space + sizeof(struct space_info) * (i + 1);
			retstat =
			    pread(fd, (char *)&temp_space_info,
				  sizeof(struct space_info), offset);
			new_node =
			    (struct space_list *)
			    malloc(sizeof(struct space_list));
			printf
			    ("***get_raid_metadata 5: i=%d, disk_id=%d, block_no=%d, retstat=%d\n",
			     i, temp_space_info.disk_id,
			     temp_space_info.disk_block_no, retstat);

			new_node->disk_id = temp_space_info.disk_id;
			new_node->disk_block_no = temp_space_info.disk_block_no;
			new_node->next = NULL;
			if (space_node != NULL) {
				space_node->next = new_node;
			} else {	//if list head is NULL
				ncfs_data->space_list_head = new_node;
			}
			space_node = new_node;
			i++;
		}
	}

	close(fd);
}

/*
 * get_disk_status: Get disk status
 *
 * @param ncfs_data: NCFS context state
 */
void FileSystemLayer::get_disk_status(struct ncfs_state *ncfs_data)
{
	int fd;
	int disk_total_num;
	int i;
	FILE *fp;

	disk_total_num = ncfs_data->disk_total_num;

	for (i = 0; i < disk_total_num; i++) {
		fd = open(ncfs_data->dev_name[i], O_RDWR);
		if (fd != -1) {
			//device is opened successfully
			ncfs_data->disk_status[i] = 0;
			printf("***get disk status: open good: i=%d\n", i);
			close(fd);
		} else {
			//fail to open device
			ncfs_data->disk_status[i] = 1;
			printf("***get disk status: open bad: i=%d\n", i);
		}
	}

	//write disk status to raid_health
	fp = fopen("raid_health", "w");
	for (i = 0; i < disk_total_num; i++) {
		fprintf(fp, "%d\n", ncfs_data->disk_status[i]);
	}
	fclose(fp);
}

/*
 * get_operation_mode: Get operation mode
 *
 * @param ncfs_data: NCFS context state
 */
void FileSystemLayer::get_operation_mode(struct ncfs_state *ncfs_data)
{
	int i;
	int disk_total_num;
	int failed_disk_num;
	int failed_disk_id;
	int disk_raid_type;
	int mode;

	disk_total_num = ncfs_data->disk_total_num;
	disk_raid_type = ncfs_data->disk_raid_type;

	failed_disk_num = 0;
	for (i = 0; i < disk_total_num; i++) {
		if (ncfs_data->disk_status[i] != 0) {
			failed_disk_num++;
		}
	}

	if (failed_disk_num == 0) {
		mode = 0;
	} else if (disk_raid_type == 1000) {	//MBR coding
		if (failed_disk_num <= (ncfs_data->mbr_n - ncfs_data->mbr_k)) {
			mode = 1;
		} else {
			mode = 2;
		}
	} else if (disk_raid_type == 3000) {	//Reed-Solomon coding
		if (failed_disk_num <=
		    (ncfs_data->disk_total_num - ncfs_data->data_disk_num)) {
			mode = 1;
		} else {
			mode = 2;
		}
	} else if (failed_disk_num == 1) {
		for (i = 0; i < disk_total_num; i++) {
			if (ncfs_data->disk_status != 0) {
				failed_disk_id = i;
			}
		}

		if (disk_raid_type == 4) {
			mode = 1;
		} else if (disk_raid_type == 5) {
			mode = 1;
		} else if (disk_raid_type == 6) {
			mode = 1;
		} else if ((disk_raid_type == 1)
			   && (i != disk_total_num - 1 - i)) {
			mode = 1;
		} else {
			mode = 2;
		}
	} else if (failed_disk_num == 2) {
		if (disk_raid_type == 6) {
			mode = 1;
		} else {
			mode = 2;
		}
	} else {
		mode = 2;
	}

	ncfs_data->operation_mode = mode;
	printf
	    ("***get_operation_mode: mode=%d, failed_disk_num=%d, disk_raid_type=%d\n",
	     mode, failed_disk_num, disk_raid_type);
}

/*
 * space_list_add: Add (deleted) space node to space list
 * 
 * @param disk_id: disk id
 * @param disk_block_no: disk block number
 * @return: space list number
 */

int FileSystemLayer::space_list_add(int disk_id, int disk_block_no)
{
	int space_list_num;
	int found;
	struct space_list *new_node;
	struct space_list *current, *prev;

	space_list_num = NCFS_DATA->space_list_num;

	new_node = (struct space_list *)malloc(sizeof(struct space_list));
	new_node->disk_id = disk_id;
	new_node->disk_block_no = disk_block_no;
	new_node->next = NULL;

	//printf("***space_list_add0: disk_id=%d, disk_block_no=%d, space_list_num=%d\n"
	//              ,disk_id,disk_block_no,space_list_num);

	if (NCFS_DATA->space_list_head == NULL) {
		//printf("***space_list_add1: first node\n");
		NCFS_DATA->space_list_head = new_node;
		space_list_num = 1;
	} else {
		//printf("***space_list_add2: list not null\n");
		current = NCFS_DATA->space_list_head;
		prev = NCFS_DATA->space_list_head;
		found = 0;
		while ((current != NULL) && (found == 0)) {
			if ((disk_id <= (current->disk_id)) &&
			    (disk_block_no < (current->disk_block_no))) {
				found = 1;
			} else {
				prev = current;
				current = current->next;
			}
		}

		if (prev == current) {	//list head
			new_node->next = NCFS_DATA->space_list_head;
			NCFS_DATA->space_list_head = new_node;
		} else {
			new_node->next = current;
			prev->next = new_node;
		}

		space_list_num++;

		//printf("***space_list_add3: space_list_num=%d\n",space_list_num);
	}

	NCFS_DATA->space_list_num = space_list_num;

	return space_list_num;
}

/*
 * space_list_remove: Remove space node from space list
 * 
 * @param disk_id: disk id
 * @param disk_block_no: disk block number
 * @return: 0 for success; -1 for error
 */
int FileSystemLayer::space_list_remove(int disk_id, int disk_block_no)
{
	int found, retstat;
	struct space_list *current, *prev;

	current = NCFS_DATA->space_list_head;
	prev = NCFS_DATA->space_list_head;

	//printf("***space_list_remove0: disk_id=%d, disk_block_no=%d\n",
	//              disk_id,disk_block_no);

	found = 0;
	if (current != NULL) {
		while ((current != NULL) && (found == 0)) {
			if ((disk_id == (current->disk_id)) &&
			    (disk_block_no == (current->disk_block_no))) {
				found = 1;
			} else {
				prev = current;
				current = current->next;
			}
		}

		if (found == 1) {
			if (prev == current) {	//list head
				NCFS_DATA->space_list_head = current->next;
			} else {
				prev->next = current->next;
			}
			free(current);
			(NCFS_DATA->space_list_num)--;
		}
	}
	//printf("***space_list_remove3: found=%d, space_list_num=%d\n",
	//              found,NCFS_DATA->space_list_num);

	if (found == 0) {
		retstat = -1;
	} else {
		retstat = 0;
	}

	return retstat;
}

void FileSystemLayer::print_device_setting(void)
{
	printf("\n\nOperation mode: %d\n", NCFS_DATA->operation_mode);
	for (int i = 0; i < NCFS_DATA->disk_total_num; ++i) {
		printf("\n Disk %d \n", i);
		printf("=============\n");
		printf("Name: %s\n", NCFS_DATA->dev_name[i]);
		printf("Free Size: %d\n", NCFS_DATA->free_size[i]);
		printf("Free Offset: %d\n", NCFS_DATA->free_offset[i]);
		printf("Status: %d\n", NCFS_DATA->disk_status[i]);
	}
	printf("=============\n");
	printf("Encoding time: %lf\n", NCFS_DATA->encoding_time);
	printf("Decoding time: %lf\n", NCFS_DATA->decoding_time);
	printf("Disk Read time: %lf\n", NCFS_DATA->diskread_time);
	printf("Disk Write time: %lf\n", NCFS_DATA->diskwrite_time);
}

int FileSystemLayer::get_fail_num(void)
{
	int count = 0;
	for (int i = 0; i < NCFS_DATA->disk_total_num; ++i) {
		if (NCFS_DATA->disk_status[i] == 1)
			++count;
	}
	return count;
}

void FileSystemLayer::set_device_status(int diskid, int status)
{
	int raid_type = NCFS_DATA->disk_raid_type;

	if ((diskid < 0) || (diskid > NCFS_DATA->disk_total_num - 1)) {
		fprintf(stderr, "Invalid Disk ID %d (0,%d)\n", diskid,
			NCFS_DATA->disk_total_num - 1);
		return;
	}
	if ((status != 0) && (status != 1)) {
		fprintf(stderr, "Invalid Status %d\n", status);
		return;
	}
	if (NCFS_DATA->disk_status[diskid] == status)
		return;
	if (NCFS_DATA->no_gui == 0) {
		diskusageLayer->ReportStatus(diskid, status);
	}
	NCFS_DATA->disk_status[diskid] = status;
	int faildisknum = get_fail_num();
	if (faildisknum == 0)
		NCFS_DATA->operation_mode = 0;
	else if ((faildisknum == 1) && ((raid_type == 1) || (raid_type == 4)
					|| (raid_type == 5)
					|| (raid_type == 6)))
		NCFS_DATA->operation_mode = 1;
	else if ((faildisknum == 2) && (raid_type == 6))
		NCFS_DATA->operation_mode = 1;
	else if ((raid_type == 1000)
		 && (faildisknum <= NCFS_DATA->mbr_n - NCFS_DATA->mbr_k))
		NCFS_DATA->operation_mode = 1;
	else if ((raid_type == 3000)
		 && (faildisknum <=
		     NCFS_DATA->disk_total_num - NCFS_DATA->data_disk_num))
		NCFS_DATA->operation_mode = 1;
	else
		NCFS_DATA->operation_mode = 2;
}

void FileSystemLayer::process_command(void)
{
	char command[256] = { 0 };
	scanf("%s", command);
	printf("-%s\n", command);
	if (strcmp(command, "recover") == 0) {
		//char* pointer = strstr(command,"recovery");
		scanf("%s|", command);
		printf("-%s|\n", command);
		//RecoveryTool::Recover(command);
		//RecoveryTool::recover();
	} else if (strcmp(command, "print") == 0) {
		print_device_setting();
	} else if (strcmp(command, "set") == 0) {
		int diskid, status;
		scanf("%d %d", &diskid, &status);
		set_device_status(diskid, status);
	} else {
		fprintf(stderr, "Invalid File Sysetm Argument: %s\n", command);
	}
	char dummy[1024];
	fgets(dummy, 1024, stdin);

}

void FileSystemLayer::update_settting(void)
{
	FILE *file;
	file = fopen("raid_setting", "w");
	fprintf(file, "disk_total_num=%d\n", NCFS_DATA->disk_total_num);
	fprintf(file, "chunk_size=%d\n", NCFS_DATA->chunk_size);
	fprintf(file, "disk_raid_type=%d\n", NCFS_DATA->disk_raid_type);
	fprintf(file, "space_list_num=0\n");
	for (int i = 0; i < NCFS_DATA->disk_total_num; ++i) {
		fprintf(file, "%d.dev_name=%s\n", i, NCFS_DATA->dev_name[i]);
		fprintf(file, "%d.free_offset=0\n", i);
		long long totallen =
		    (NCFS_DATA->free_offset[i] +
		     NCFS_DATA->free_size[i]) * NCFS_DATA->chunk_size;
		fprintf(file, "%d.free_size=%lld\n", i, totallen);
	}
	fclose(file);
}

//Add by zhuyunfeng on October 11, 2011 begin.
void FileSystemLayer::readSystemConfig(struct ncfs_state* ncfs_data)
{
	ncfs_data->disk_total_num = configLayer->getConfigInt("FileSystem>Disk>TotalDiskNumber");	
	ncfs_data->data_disk_num = configLayer->getConfigInt("FileSystem>Disk>DataDiskNumber");
	ncfs_data->chunk_size = configLayer->getConfigLong("FileSystem>Disk>ChunkSize");
	ncfs_data->disk_raid_type = configLayer->getConfigInt("FileSystem>Disk>RaidType");
	ncfs_data->no_cache = configLayer->getConfigInt("FileSystem>Setting>NoCache");
	ncfs_data->no_gui = configLayer->getConfigInt("FileSystem>Setting>NoGui");
	ncfs_data->run_experiment = configLayer->getConfigInt("FileSystem>Setting>Experiment");
	ncfs_data->disk_size = (int*)malloc(sizeof(int)*ncfs_data->disk_total_num);
	ncfs_data->free_offset = (int*)malloc(sizeof(int)*ncfs_data->disk_total_num);
	ncfs_data->free_size = (int*)malloc(sizeof(int)*ncfs_data->disk_total_num);
	ncfs_data->disk_status = (int*)malloc(sizeof(int)*ncfs_data->disk_total_num);
	ncfs_data->dev_name = (char **)calloc(ncfs_data->disk_total_num, sizeof(char *));
	
	char targetDisk[50];
	char tempString[256];
	for(int i = 0; i < ncfs_data->disk_total_num; ++i){
		memset(targetDisk,0,50);
		sprintf(targetDisk,"FileSystem>Disk>DiskSetting>Disk%d>",i+1);
		memset(tempString,0,256);
		sprintf(tempString,"%sDevName",targetDisk);
		const char* devNameTemp = configLayer->getConfigString(tempString);
		ncfs_data->dev_name[i] = (char*)calloc(PATH_MAX,1);
		strncpy(ncfs_data->dev_name[i],devNameTemp,PATH_MAX);
		memset(tempString,0,256);
		sprintf(tempString,"%sTotalSize",targetDisk);
		ncfs_data->disk_size[i] = configLayer->getConfigInt(tempString);
		memset(tempString,0,256);
		sprintf(tempString,"%sFreeOffset",targetDisk);
		ncfs_data->free_offset[i] = configLayer->getConfigLong(tempString);
		memset(tempString,0,256);
		sprintf(tempString,"%sFreeSize",targetDisk);
		ncfs_data->free_size[i] = configLayer->getConfigLong(tempString);
		
	}
	
}
//Add by zhuyunfeng on October 11, 2011 end.