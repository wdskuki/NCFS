/***
Raid setup tool for ncfs

This would read the raid_setting and raid_health files to setup the raid.
The data in devices listed in raid_setting would be cleared.

Usage:
sudo ./setup
***/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

struct ncfs_state {
	char *rootdir;

	//start ncfs
	int disk_total_num;
	int disk_block_size;
	int disk_raid_type;
	int space_list_num;
	int *free_offset;	//in block number
	int *free_size;		//in number of blocks
	char **dev_name;

	//end ncfs
};

enum param_type {
	illegal_param, disk_total_num, disk_block_size, disk_raid_type,
	    space_list_num,
	free_offset, free_size, dev_name
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

int parse_setting_line(struct ncfs_state *ncfs_data, const char *line)
{
	global_param_int(ncfs_data, line, disk_total_num, "disk_total_num");
	global_param_int(ncfs_data, line, disk_block_size, "disk_block_size");
	global_param_int(ncfs_data, line, disk_raid_type, "disk_raid_type");
	//global_param_int(ncfs_data, line, space_list_num, "space_list_num");
	disk_param_int(ncfs_data, line, free_offset, "free_offset");
	disk_param_int(ncfs_data, line, free_size, "free_size");
	disk_param_string(ncfs_data, line, dev_name, "dev_name");
	return illegal_param;
}

void get_raid_setting(struct ncfs_state *ncfs_data)
{
	FILE *fp = fopen("raid_setting", "r");
	char buf[80];
	while (fgets(buf, sizeof(buf), fp)) {
		if (parse_setting_line(ncfs_data, buf) == disk_total_num) {
			ncfs_data->free_offset =
			    (int *)malloc(sizeof(int) *
					  ncfs_data->disk_total_num);
			ncfs_data->free_size =
			    (int *)malloc(sizeof(int) *
					  ncfs_data->disk_total_num);
			ncfs_data->dev_name =
			    (char **)calloc(ncfs_data->disk_total_num,
					    sizeof(char *));
		}
	}
	fclose(fp);
}

//clear data of the device
//Note size = number of blocks
int clear_data(const char *dev, int size, int block_size)
{
	int fd;
	long long offset;
	char buf[block_size];
	int retstat;
	int i;

	printf("***clear_data0: dev=%s, size=%d, block_size=%d\n",
	       dev, size, block_size);

	memset(buf, 0, block_size);

	fd = open(dev, O_WRONLY);
	printf("***clear_data1: fd=%d\n", fd);

	offset = 0;
	for (i = 0; i < size; i++) {
		retstat = pwrite(fd, buf, block_size, offset);
		offset = offset + block_size;
	}

	close(fd);

	printf("***clear_data2: retstat=%d\n", retstat);

	return retstat;
}

int main(int argc, char *argv[])
{
	struct ncfs_state *ncfs_data;
	char *target_dev;
	int *raid_health;
	int disk_total_num;
	int i, j;
	FILE *health_fp;
	char buf[3];
	int fail_disk_num;
	int *fail_disk_id;

	ncfs_data = calloc(sizeof(struct ncfs_state), 1);

	get_raid_setting(ncfs_data);
	for (j = 0; j < ncfs_data->disk_total_num; j++) {
		ncfs_data->free_offset[j] = (ncfs_data->free_offset[j])
		    / (ncfs_data->disk_block_size);
		ncfs_data->free_size[j] = (ncfs_data->free_size[j])
		    / (ncfs_data->disk_block_size);
	}

	//test print
	printf("***main: disk_total_num=%d, block_size=%d, raid_type=%d\n",
	       ncfs_data->disk_total_num, ncfs_data->disk_block_size,
	       ncfs_data->disk_raid_type);
	for (j = 0; j < ncfs_data->disk_total_num; j++) {
		printf("***main: j=%d, dev=%s, free_offset=%d, free_size=%d\n",
		       j, ncfs_data->dev_name[j], ncfs_data->free_offset[j],
		       ncfs_data->free_size[j]);
	}

	disk_total_num = ncfs_data->disk_total_num;
	raid_health = (int *)malloc(sizeof(int) * (disk_total_num));

	target_dev = argv[1];

	health_fp = fopen("raid_health", "r");
	fail_disk_num = 0;
	fail_disk_id = (int *)malloc(sizeof(int) * disk_total_num);
	for (i = 0; i < disk_total_num; i++) {
		fgets(buf, sizeof(buf), health_fp);
		raid_health[i] = atoi(buf);
		printf("raid health: disk_id=%d, status=%d\n", i,
		       raid_health[i]);

		if (raid_health[i] != 0) {	//1 for fail; 0 for health
			fail_disk_id[fail_disk_num] = i;
			fail_disk_num++;
		}
	}

	fclose(health_fp);

	for (i = 0; i < disk_total_num; i++) {
		clear_data(ncfs_data->dev_name[i], ncfs_data->free_size[i],
			   ncfs_data->disk_block_size);
	}

	return 0;
}
