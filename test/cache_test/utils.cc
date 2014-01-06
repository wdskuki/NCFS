#include "utils.hh"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>

using namespace filesystem;

//ECFS context state
extern struct ecfs_state* ECFS_DATA;

enum param_type
{
	illegal_param, disk_total_num, disk_block_size, disk_raid_type, space_list_num, 
	free_offset, free_size, dev_name
};

#define is_param(line, param_str)                                       \
	(strncmp((line), (param_str), sizeof(param_str) - 1) == 0)

#define global_param_int(ecfs_data, line, param, param_str)               \
	do {                                                                \
		if (is_param((line), (param_str))) {                            \
			(ecfs_data)->param = atoi((line) + sizeof(param_str));        \
			return param;                                               \
		}                                                               \
	} while(0)

#define disk_param_int(ecfs_data, line, param, param_str)                 \
	do {                                                                \
		char *dot = strchr(line, '.');                                  \
		if (dot && is_param(dot + 1, (param_str))) {                    \
			(ecfs_data)->param[atoi(line)] = atoi((dot + 1 + sizeof(param_str))); \
			return param;                                               \
		}                                                               \
	} while(0)

#define disk_param_string(ecfs_data, line, param, param_str)              \
	do {                                                                \
		char *dot = strchr(line, '.');                                  \
		if (dot && is_param(dot + 1, (param_str))) {                    \
			char *val = dot + 1 + sizeof(param_str);                    \
			size_t len = strlen(val);                                   \
			char *dev = (char *)calloc(len, sizeof(char));              \
			strncpy(dev, val, len - 1);                                 \
			(ecfs_data)->param[atoi((line))] = dev;                       \
			return param;                                               \
		}                                                               \
	} while(0)


/*
parse_setting_line: Parse a line in setting file

@param ecfs_data: ECFS context state
@param line: a line in setting file

@return: param_type
*/
int FS_manager::parse_setting_line(struct ecfs_state *ecfs_data, char *line)
{
	global_param_int(ecfs_data, line, disk_total_num, "disk_total_num");
	global_param_int(ecfs_data, line, disk_block_size, "disk_block_size");
	global_param_int(ecfs_data, line, disk_raid_type, "disk_raid_type");
	//global_param_int(ecfs_data, line, space_list_num, "space_list_num");
	disk_param_int(ecfs_data, line, free_offset, "free_offset");
	disk_param_int(ecfs_data, line, free_size, "free_size");
	disk_param_string(ecfs_data, line, dev_name, "dev_name");
	return illegal_param;
}


/*
FS_manager constructor
*/
FS_manager::FS_manager()
{

}


//  All the paths I see are relative to the root of the mounted
//  filesystem.  In order to get to the underlying filesystem, I need to
//  have the mountpoint.  I'll save it away early on in main(), and then
//  whenever I need a path for something I'll call this to construct
//  it.
/*
ecfs_fullpath: get full path of a file

@param fpath: output full path
@param path: file path
*/
void FS_manager::ecfs_fullpath(char fpath[PATH_MAX], const char *path)
{
	strcpy(fpath, ECFS_DATA->rootdir);
	strncat(fpath, path, PATH_MAX); // ridiculously long paths will
	return ;
	// break here
}

/*
search_disk_id: Search for disk id provided by device path

@param path: device path

@return: 
*/
int FS_manager::search_disk_id(const char* path)
{
	int i;
	for(i = 0; i < ECFS_DATA->disk_total_num; ++i){
		if(strcmp(ECFS_DATA->dev_name[i],path) == 0)
			return i;	
	}
	return -1;
}

/*
get_disk_dev_name: Get a disk id's device name

@param disk_id: disk ID

@return: device name
*/
char* FS_manager::get_disk_dev_name(int disk_id)
{
	return ECFS_DATA->dev_name[disk_id];
}

/*
round_to_block_size: Adjust size to around n block sizes

@param size: size in bytes

@return: rounded size in bytes
*/
int FS_manager::round_to_block_size(int size)
{
	int result;
	int disk_block_size;

	disk_block_size = ECFS_DATA->disk_block_size;

        if ((size % disk_block_size) != 0){
                result = size + disk_block_size - (size % disk_block_size);
        }
	else{
		result = size;
	}

	return result;
}

/*
get_raid_setting: Get ECFS setting from setting file

@param ecfs_data: ECFS context state
*/
void FS_manager::get_raid_setting(struct ecfs_state *ecfs_data)
{
	FILE *fp = fopen("raid_setting", "r");
	char buf[80];
	while (fgets(buf, sizeof(buf), fp)) {
		if (parse_setting_line(ecfs_data, buf) == disk_total_num) {
			ecfs_data->free_offset = (int*)malloc(sizeof(int)*ecfs_data->disk_total_num);
			ecfs_data->free_size = (int*)malloc(sizeof(int)*ecfs_data->disk_total_num);
			ecfs_data->disk_status = (int*)malloc(sizeof(int)*ecfs_data->disk_total_num);
			ecfs_data->dev_name = (char **)calloc(ecfs_data->disk_total_num, sizeof(char *));
		}
	}
	fclose(fp);
}


/*
get_raid_metadata: Get ECFS metadata from metadata file

@param ecfs_data: ECFS context state
*/
void FS_manager::get_raid_metadata(struct ecfs_state *ecfs_data)
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

	fd = open("raid_metadata",O_RDONLY);    
	retstat = pread(fd, (char*)&magic_no, sizeof(int), 0);

	printf("***get_raid_metadata 0: magic_no = %d\n",magic_no);

	if (magic_no == MAGIC_NUMBER){

		printf("***get_raid_metadata 1: magic_no = %d\n",magic_no);

		//get disk free size info
		for (i=0; i < ecfs_data->disk_total_num; i++){
			offset = sizeof(struct raid_metadata)*(i+1);
			retstat = pread(fd, (char*)&metadata, sizeof(struct raid_metadata), 
					offset);
			if (retstat > 0){
				printf("***get_raid_metadata 3: offset = %d, disk_id=%d,free_offset=%d, free_size=%d\n",offset,metadata.disk_id,metadata.free_offset,metadata.free_size);

				disk_id = metadata.disk_id;
				ecfs_data->free_offset[disk_id] = metadata.free_offset;
				ecfs_data->free_size[disk_id] = metadata.free_size;
			}
		}

		//get disk space list info (deleted spaces)
		offset_space = sizeof(struct raid_metadata)*(ecfs_data->disk_total_num + 1);
		retstat = pread(fd, (char*)&space_list_num, sizeof(int), offset_space);
		ecfs_data->space_list_num = space_list_num;
		printf("***get_raid_metadata 4: space_list_num = %d\n",space_list_num);

		i = 0;
		space_node = ecfs_data->space_list_head;
		while ((i < space_list_num) && (retstat > 0)){
			offset = offset_space + sizeof(struct space_info)*(i+1);
			retstat = pread(fd, (char*)&temp_space_info, sizeof(struct space_info),
					offset);
			new_node = (struct space_list *)malloc(sizeof(struct space_list));
			printf("***get_raid_metadata 5: i=%d, disk_id=%d, block_no=%d, retstat=%d\n",
					i,temp_space_info.disk_id,temp_space_info.disk_block_no,retstat);

			new_node->disk_id = temp_space_info.disk_id;
			new_node->disk_block_no = temp_space_info.disk_block_no;
			new_node->next = NULL;
			if (space_node != NULL){
				space_node->next = new_node;
			}
			else{	//if list head is NULL
				ecfs_data->space_list_head = new_node;
			}
			space_node = new_node;
			i++;
		}
	}

	close(fd);
}


/*
get_disk_status: Get disk status

@param ecfs_data: ECFS context state
*/
void FS_manager::get_disk_status(struct ecfs_state *ecfs_data)
{
	int fd;
	int disk_total_num;
	int i;
	FILE *fp;

	disk_total_num = ecfs_data->disk_total_num;

	for (i=0; i < disk_total_num; i++){	
		fd = open(ecfs_data->dev_name[i], O_RDWR);
		if (fd != -1){
			//device is opened successfully
			ecfs_data->disk_status[i] = 0;                
			printf("***get disk status: open good: i=%d\n",i);
			close(fd);
		}
		else{
			//fail to open device
			ecfs_data->disk_status[i] = 1;
			printf("***get disk status: open bad: i=%d\n",i);
		}	
	}

	//write disk status to raid_health
	fp = fopen("raid_health","w");
	for (i=0; i < disk_total_num; i++){
		fprintf(fp,"%d\n",ecfs_data->disk_status[i]);
	}
	fclose(fp);
}


/*
get_operation_mode: Get operation mode

@param ecfs_data: ECFS context state
*/
void FS_manager::get_operation_mode(struct ecfs_state *ecfs_data)
{
	int i;
	int disk_total_num;
	int failed_disk_num;
	int failed_disk_id;
	int disk_raid_type;
	int mode;

	disk_total_num = ecfs_data->disk_total_num;
	disk_raid_type = ecfs_data->disk_raid_type;

	failed_disk_num = 0;
	for (i=0; i < disk_total_num; i++){
		if (ecfs_data->disk_status[i] != 0){
			failed_disk_num++;
		}
	}

        if (failed_disk_num == 0){
                mode = 0;
        }
	else if (failed_disk_num == 1){
		for (i=0; i < disk_total_num; i++){
			if (ecfs_data->disk_status != 0){
				failed_disk_id = i;
			}
		}

		if (disk_raid_type == 4){
			mode = 1;
		}
		else if (disk_raid_type == 5){
			mode = 1;
		}
		else if ((disk_raid_type == 1) && (i != disk_total_num - 1 - i)){
			mode = 1;
		}
		else{
			mode = 2;
		}			
	}
	else{
		mode = 2;
	}

	ecfs_data->operation_mode = mode;
	printf("***get_operation_mode: mode=%d, failed_disk_num=%d, disk_raid_type=%d\n",mode,failed_disk_num,disk_raid_type);
}
