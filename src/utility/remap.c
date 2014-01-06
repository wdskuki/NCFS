/***
  Recovery tool for ncfs

  Support remapping of raid 5
  It checks the raid_health file for status of the disks.
Usage:
sudo ./remap
 ***/

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

struct bb_state {
	FILE *logfile;
	char *rootdir;

	//start ncfs
	int disk_total_num;
	int disk_block_size;
	int disk_raid_type;
	int space_list_num;
	int *free_offset;	//in block number
	int *free_size;		//in number of blocks
	char **dev_name;

	//int free_offset;
	//int free_size;
	//end ncfs
};

struct raid_metadata {
	int disk_id;
	int free_offset;
	int free_size;
};

struct data_block_info {
	int disk_id;
	int block_no;
};

struct block_move {
	struct data_block_info ori;
	struct data_block_info new;
};

struct block_move move_rec[100];
int move_cnt = 0;
char rootdir[] = "rootdir";

enum param_type {
	illegal_param, disk_total_num, disk_block_size, disk_raid_type,
	    space_list_num,
	free_offset, free_size, dev_name
};

#define is_param(line, param_str)                                       \
	(strncmp((line), (param_str), sizeof(param_str) - 1) == 0)

#define global_param_int(bb_data, line, param, param_str)               \
	do {                                                                \
		if (is_param((line), (param_str))) {                            \
			(bb_data)->param = atoi((line) + sizeof(param_str));        \
			return param;                                               \
		}                                                               \
	} while(0)

#define disk_param_int(bb_data, line, param, param_str)                 \
	do {                                                                \
		char *dot = strchr(line, '.');                                  \
		if (dot && is_param(dot + 1, (param_str))) {                    \
			(bb_data)->param[atoi(line)] = atoi((dot + 1 + sizeof(param_str))); \
			return param;                                               \
		}                                                               \
	} while(0)

#define disk_param_string(bb_data, line, param, param_str)              \
	do {                                                                \
		char *dot = strchr(line, '.');                                  \
		if (dot && is_param(dot + 1, (param_str))) {                    \
			char *val = dot + 1 + sizeof(param_str);                    \
			size_t len = strlen(val);                                   \
			char *dev = (char *)calloc(len, sizeof(char));              \
			strncpy(dev, val, len - 1);                                 \
			(bb_data)->param[atoi((line))] = dev;                       \
			return param;                                               \
		}                                                               \
	} while(0)

int parse_setting_line(struct bb_state *bb_data, const char *line)
{
	global_param_int(bb_data, line, disk_total_num, "disk_total_num");
	global_param_int(bb_data, line, disk_block_size, "disk_block_size");
	global_param_int(bb_data, line, disk_raid_type, "disk_raid_type");
	//global_param_int(bb_data, line, space_list_num, "space_list_num");
	disk_param_int(bb_data, line, free_offset, "free_offset");
	disk_param_int(bb_data, line, free_size, "free_size");
	disk_param_string(bb_data, line, dev_name, "dev_name");
	return illegal_param;
}

void get_raid_setting(struct bb_state *bb_data)
{
	FILE *fp = fopen("raid_setting", "r");
	char buf[80];
	while (fgets(buf, sizeof(buf), fp)) {
		if (parse_setting_line(bb_data, buf) == disk_total_num) {
			bb_data->free_offset =
			    (int *)malloc(sizeof(int) *
					  bb_data->disk_total_num);
			bb_data->free_size =
			    (int *)malloc(sizeof(int) *
					  bb_data->disk_total_num);
			bb_data->dev_name =
			    (char **)calloc(bb_data->disk_total_num,
					    sizeof(char *));
		}
	}
	fclose(fp);
}

void get_raid_metadata(struct bb_state *bb_data)
{
	struct raid_metadata metadata;
	int fd = open("raid_metadata", O_RDONLY);
	int magic_no;
	int retstat = pread(fd, (char *)&magic_no, sizeof(int), 0);
	int i;
	if (retstat <= 0)
		return;
	else {
		for (i = 0; i < bb_data->disk_total_num; i++) {
			int offset = sizeof(struct raid_metadata) * (i + 1);
			retstat =
			    pread(fd, (char *)&metadata,
				  sizeof(struct raid_metadata), offset);
			bb_data->free_offset[i] = metadata.free_offset;
			bb_data->free_size[i] = metadata.free_size;
		}

	}
}

void update_raid_metadata(struct bb_state *bb_data, int fail_disk)
{
	int fd = open("raid_metadata", O_RDONLY);
	int magic_no;
	int retstat = pread(fd, (char *)&magic_no, sizeof(int), 0);
	int i;
	if (retstat <= 0)
		return;
	else {
		close(fd);
		int fd = open("raid_metadata", O_WRONLY | O_TRUNC);
		pwrite(fd, (char *)&magic_no, sizeof(int), 0);
		int disk_total_num = bb_data->disk_total_num;
		int block_size = bb_data->disk_block_size;
		for (i = 0; i < disk_total_num; i++) {
			struct raid_metadata metadata;
			if (i == fail_disk)
				continue;
			metadata.disk_id = i;
			if (i > fail_disk)
				metadata.disk_id--;
			metadata.free_offset = bb_data->free_offset[i];
			metadata.free_size =
			    (bb_data->free_size[i]) * block_size;
			int offset =
			    sizeof(struct raid_metadata) * (metadata.disk_id +
							    1);
			pwrite(fd, (char *)&metadata,
			       sizeof(struct raid_metadata), offset);
			printf
			    ("Update_metadata: id = %d, free_offset = %d, free_size = %d\n",
			     metadata.disk_id, metadata.free_offset,
			     metadata.free_size);
		}
		close(fd);
	}
}

void update_raid_health(struct bb_state *bb_data)
{
	FILE *file = fopen("raid_health", "w");
	int i;
	for (i = 0; i < bb_data->disk_total_num - 1; i++)
		fprintf(file, "0\n");
	fclose(file);
}

void update_raid_setting(struct bb_state *bb_data, int fail_disk)
{
	FILE *file = fopen("raid_setting", "w");
	int i;
	fprintf(file, "disk_total_num=%d\n", bb_data->disk_total_num - 1);
	fprintf(file, "disk_block_size=%d\n", bb_data->disk_block_size);
	fprintf(file, "disk_raid_type=%d\n", bb_data->disk_raid_type);
	fprintf(file, "space_list_num=0");
	for (i = 0; i < bb_data->disk_total_num; i++) {
		if (i == fail_disk)
			continue;
		int id = i;
		if (i > fail_disk)
			id--;
		fprintf(file, "\n%d.dev_name=%s", id, bb_data->dev_name[i]);
		fprintf(file, "\n%d.free_offset=%d", id,
			bb_data->free_offset[i]);
		fprintf(file, "\n%d.free_size=%d", id,
			bb_data->free_size[i] * bb_data->disk_block_size);
	}
	fclose(file);
}

int find_space(struct bb_state *bb_data, int fail_disk)
{
	int i;
	int maxsize = -1;
	int disk = -1;
	int parity_disk_id;
	for (i = 0; i < bb_data->disk_total_num; i++) {
		if (i == fail_disk)
			continue;
		parity_disk_id =
		    bb_data->disk_total_num - 2 -
		    (bb_data->free_offset[i] % (bb_data->disk_total_num - 1));
		if (i == parity_disk_id)
			continue;
		if (bb_data->free_size[i] > maxsize) {
			maxsize = bb_data->free_size[i];
			disk = i;
		}
	}
	if (disk == -1) {
		printf("Space: Error, cannot allocate space\n");
	} else {
		printf("Space: Space found at disk = %d, block = %d\n", disk,
		       bb_data->free_offset[disk]);
	}
	return disk;
}

void xor_encoding(struct bb_state *bb_data, int block_no, int fail_disk)
{
	int new_disk_total_num = bb_data->disk_total_num - 1;
	int parity_disk_id =
	    new_disk_total_num - 1 - (block_no % new_disk_total_num);
	if (parity_disk_id >= fail_disk)
		parity_disk_id++;
	int j, k;
	int block_size = bb_data->disk_block_size;
	char *buf, *temp_buf;
	buf = (char *)malloc(sizeof(char) * block_size);
	temp_buf = (char *)malloc(sizeof(char) * block_size);
	memset(buf, 0, block_size);
	memset(temp_buf, 0, block_size);

	int offset = block_no * block_size;
	for (j = 0; j < bb_data->disk_total_num; j++) {
		if ((fail_disk != j) && (parity_disk_id != j)) {
			char *dev = bb_data->dev_name[j];
			int datafd = open(dev, O_RDONLY);
			pread(datafd, temp_buf, block_size, offset);
			printf
			    ("Xor_encoding: Read from %s, size = %d, offset = %d\n",
			     dev, block_size, offset);
			for (k = 0; k < block_size; k++) {
				buf[k] = buf[k] ^ temp_buf[k];
			}
			close(datafd);
		}
	}
	char *dev = bb_data->dev_name[parity_disk_id];
	int datafd = open(dev, O_WRONLY);
	pwrite(datafd, buf, block_size, offset);
	printf("Xor_encoding: Write to %s, size = %d, offset = %d\n", dev,
	       block_size, offset);
	if (block_no > bb_data->free_offset[parity_disk_id]) {
		bb_data->free_size -=
		    (block_no - bb_data->free_offset[parity_disk_id]);
		bb_data->free_offset[parity_disk_id] = block_no;
	}
	close(datafd);
}

void update_xor_encoding(struct bb_state *bb_data, int fail_disk)
{
	int i;
	int maxused = -1;
	for (i = 0; i < bb_data->disk_total_num; i++) {
		if (i == fail_disk)
			continue;
		if (bb_data->free_offset[i] > maxused) {
			maxused = bb_data->free_offset[i];
		}
	}
	if (maxused != -1)
		for (i = 0; i < maxused; i++)
			xor_encoding(bb_data, i, fail_disk);
	return;

}

int write_and_allocate(struct bb_state *bb_data, char *buf, int fail_disk)
{
	int disk_to_write = find_space(bb_data, fail_disk);
	int block_to_write = bb_data->free_offset[disk_to_write];
	char *dev = bb_data->dev_name[disk_to_write];
	int datafd = open(dev, O_WRONLY);
	int block_size = bb_data->disk_block_size;
	int retstat =
	    pwrite(datafd, buf, block_size, block_to_write * block_size);
	printf("Write and allocate: Write to %s, size = %d, offset = %d\n", dev,
	       block_size, block_to_write * block_size);
//      printf("/////Write Content\\\\\\\\\\\n");
//      int k;
//      for (k = 0;k < block_size; k++)
//              printf("%c",buf[k]);
//      printf("\\\\\\\\\\Write Content/////\n");

	bb_data->free_offset[disk_to_write]++;
	bb_data->free_size[disk_to_write]--;
	printf("Write and allocate: %s free_offset = %d, free_size = %d\n", dev,
	       bb_data->free_offset[disk_to_write],
	       bb_data->free_size[disk_to_write]);
	move_rec[move_cnt].new.disk_id = disk_to_write;
	if (disk_to_write > fail_disk)
		move_rec[move_cnt].new.disk_id--;
	move_rec[move_cnt].new.block_no = block_to_write;
	//xor_encoding(bb_data,block_to_write,fail_disk);
	return retstat;
}

//Adjust size to around n block sizes
int round_to_block_size(struct bb_state *bb_data, int size)
{
	int result;
	int disk_block_size;

	disk_block_size = bb_data->disk_block_size;

	if ((size % disk_block_size) != 0) {
		result = size + disk_block_size - (size % disk_block_size);
	} else {
		result = size;
	}

	return result;
}

int block_info_match(struct data_block_info a, struct data_block_info b)
{
	return ((a.disk_id == b.disk_id) && (a.block_no == b.block_no));
}

void print_move_rec(struct block_move *move_recc, int count)
{
	int i;
	printf("==========print_move_rec==========\n");
	for (i = 0; i < count; i++) {
		printf("/////ori\\\\\\\\\\\n");
		printf("disk %d, block %d\n", move_recc[i].ori.disk_id,
		       move_recc[i].ori.block_no);
		printf("\\\\\\\\\\ori/////\n");

		printf("/////new\\\\\\\\\\\n");
		printf("disk %d, block %d\n", move_recc[i].new.disk_id,
		       move_recc[i].new.block_no);
		printf("\\\\\\\\\\new/////\n");
	}
	printf("==========print_move_rec==========\n");
}

void update_file_block(struct bb_state *bb_data)
{
	DIR *dir;
	struct dirent *ptr;
	int i, j;
	dir = opendir(rootdir);
	ptr = readdir(dir);
	ptr = readdir(dir);
	// Read out . and .. entries
	while ((ptr = readdir(dir)) != NULL) {
		char filename[80];
		strcpy(filename, rootdir);
		strcat(filename, "/");
		strcat(filename, ptr->d_name);
		struct data_block_info temp_block_info;
		int fd = open(filename, O_RDWR);
		int retstat;
		int file_size;
		retstat = pread(fd, (char *)&file_size, sizeof(int), 0);
		printf("retstat = %d\n", retstat);
		printf("Updating File %s\n", filename);
		printf("File size = %d\n", file_size);
		file_size = round_to_block_size(bb_data, file_size);
		for (i = 0; i < file_size / bb_data->disk_block_size; i++) {
			int addr_offset =
			    (i + 1) * sizeof(struct data_block_info);
			pread(fd, (char *)&temp_block_info,
			      sizeof(struct data_block_info), addr_offset);
			printf
			    ("Checking block %d, disk_id = %d, block_no = %d\n",
			     i, temp_block_info.disk_id,
			     temp_block_info.block_no);
			for (j = 0; j < move_cnt; j++) {
				if (block_info_match
				    (temp_block_info, move_rec[j].ori) == 1) {
					pwrite(fd, (char *)&move_rec[j].new,
					       sizeof(struct data_block_info),
					       addr_offset);
					printf
					    ("Point to new block, disk_id = %d, block_no = %d\n",
					     move_rec[j].new.disk_id,
					     move_rec[j].new.block_no);
					break;
				}
			}
		}
		close(fd);
	}
	return;
}

/*
remap_xor: Remap for raid 5

@param bb_data: NCFS context state
@param fail_disk: fail disk id
*/
int remap_xor(struct bb_state *bb_data, int fail_disk)
{

	int block_size, disk_size;
	int free_offset;
	int offset;
	int i, j, k;
	int ori_parity_disk_id;
	int new_parity_disk_id;
	int new_disk_total_num = bb_data->disk_total_num - 1;
	char *buf, *temp_buf;

	printf("remap_xor: fail_disk_id=%d\n", fail_disk);

	block_size = bb_data->disk_block_size;
	printf("remap_xor: block_size=%d\n", block_size);
	buf = (char *)malloc(sizeof(char) * block_size);
	temp_buf = (char *)malloc(sizeof(char) * block_size);

	disk_size = bb_data->free_size[fail_disk];
	printf("remap_xor: disk_size=%d\n", disk_size);

	free_offset = bb_data->free_offset[fail_disk];
	printf("remap_xor: free_offset=%d\n", free_offset);

	for (i = 0; i < free_offset; i++) {
		printf("Block %d\n", i);
		offset = i * block_size;
		memset(buf, 0, block_size);
		memset(temp_buf, 0, block_size);

		ori_parity_disk_id =
		    bb_data->disk_total_num - 1 - (i % bb_data->disk_total_num);
		new_parity_disk_id =
		    new_disk_total_num - 1 - (i % new_disk_total_num);
		if (new_parity_disk_id >= fail_disk)
			new_parity_disk_id++;
		printf("Parity disk, %d -> %d\n", ori_parity_disk_id,
		       new_parity_disk_id);
		int disk_to_be_moved = -1;
		int recover = 1;
		if (ori_parity_disk_id == fail_disk) {
			recover = 0;
			disk_to_be_moved = new_parity_disk_id;
		} else {
			if (recover == 1) {
				memset(buf, 0, block_size);
				memset(temp_buf, 0, block_size);
				for (j = 0; j < bb_data->disk_total_num; j++) {
					if (fail_disk != j) {
						char *dev =
						    bb_data->dev_name[j];
						int datafd =
						    open(dev, O_RDONLY);
						pread(datafd, temp_buf,
						      block_size, offset);
						printf
						    ("Recover: Read from %s, size = %d, offset = %d\n",
						     dev, block_size, offset);
						for (k = 0; k < block_size; k++) {
							buf[k] =
							    buf[k] ^
							    temp_buf[k];
						}
						close(datafd);
					}
				}
				move_rec[move_cnt].ori.disk_id = fail_disk;
				move_rec[move_cnt].ori.block_no = i;
				write_and_allocate(bb_data, buf, fail_disk);
				move_cnt++;
				print_move_rec(move_rec, move_cnt);
			}
			if (new_parity_disk_id == ori_parity_disk_id) {
				printf("Generate partity block\n");
			} else {
				printf
				    ("Swap data block with new parity block\n");
				char *dev =
				    bb_data->dev_name[new_parity_disk_id];
				int datafd = open(dev, O_RDONLY);
				int retstat =
				    pread(datafd, buf, block_size, offset);
				printf
				    ("Swap: Read from %s\n, size = %d, offset = %d\n",
				     dev, block_size, offset);
				close(datafd);
				dev = bb_data->dev_name[ori_parity_disk_id];
				datafd = open(dev, O_WRONLY);
				retstat =
				    pwrite(datafd, buf, block_size, offset);
				printf
				    ("Swap: Write to %s\n, size = %d, offset = %d\n",
				     dev, block_size, offset);
				close(datafd);
				move_rec[move_cnt].ori.disk_id =
				    new_parity_disk_id;
				move_rec[move_cnt].ori.block_no = i;
				move_rec[move_cnt].new.disk_id =
				    ori_parity_disk_id;
				if (ori_parity_disk_id > fail_disk)
					move_rec[move_cnt].new.disk_id--;
				move_rec[move_cnt].new.block_no = i;
				move_cnt++;
				print_move_rec(move_rec, move_cnt);
			}
			//xor_encoding(bb_data,i,fail_disk);
		}
		if (disk_to_be_moved != -1) {
			memset(buf, 0, block_size);
			memset(temp_buf, 0, block_size);
			char *dev = bb_data->dev_name[disk_to_be_moved];
			int datafd = open(dev, O_RDONLY);
			pread(datafd, buf, block_size, offset);
			printf("Move: Read from %s, size = %d, offset = %d\n",
			       dev, block_size, offset);
			close(datafd);
			move_rec[move_cnt].ori.disk_id = disk_to_be_moved;
			move_rec[move_cnt].ori.block_no = i;
			write_and_allocate(bb_data, buf, fail_disk);
			move_cnt++;
			print_move_rec(move_rec, move_cnt);
		}
	}
	update_xor_encoding(bb_data, fail_disk);
	update_file_block(bb_data);
	update_raid_metadata(bb_data, fail_disk);
	update_raid_health(bb_data);
	update_raid_setting(bb_data, fail_disk);
	return 0;
}

int main(int argc, char *argv[])
{
	struct bb_state *bb_data;
	int *raid_health;
	int disk_total_num;
	int i, j;
	FILE *health_fp;
	char buf[3];
	int fail_disk_num;
	int *fail_disk_id;

	bb_data = calloc(sizeof(struct bb_state), 1);

	get_raid_setting(bb_data);
	get_raid_metadata(bb_data);
	for (j = 0; j < bb_data->disk_total_num; j++) {
		bb_data->free_offset[j] = (bb_data->free_offset[j]);
		bb_data->free_size[j] = (bb_data->free_size[j])
		    / (bb_data->disk_block_size);
	}

	//test print
	printf("***main: disk_total_num=%d, block_size=%d, raid_type=%d\n",
	       bb_data->disk_total_num, bb_data->disk_block_size,
	       bb_data->disk_raid_type);
	for (j = 0; j < bb_data->disk_total_num; j++) {
		printf("***main: j=%d, dev=%s, free_offset=%d, free_size=%d\n",
		       j, bb_data->dev_name[j], bb_data->free_offset[j],
		       bb_data->free_size[j]);
	}

	disk_total_num = bb_data->disk_total_num;
	raid_health = (int *)malloc(sizeof(int) * (disk_total_num));

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

	printf("fail_disk_num = %d\n", fail_disk_num);
	for (i = 0; i < fail_disk_num; i++)
		printf("fail_disk_id  = %d\n", fail_disk_id[i]);
	if (fail_disk_num > 0) {
		if ((disk_total_num - fail_disk_num) < 3)
			printf("At least 3 healthy disks are needed\n");
		else if (fail_disk_num == 1) {
			if (bb_data->disk_raid_type == 5) {
				printf("Going to remap.\n");
				remap_xor(bb_data, fail_disk_id[0]);
			} else {
				printf("Only Raid 5 is supported\n");
			}
		} else {
			printf("Cannot remap, more than one disk failed\n");
		}
	} else {
		printf("No disk fails.\n");
	}
	return 0;
}
