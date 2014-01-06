/***
Recovery tool for ncfs

Support recovery of raid 1,4,5
It checks the raid_health file for status of the disks.
Usage:
sudo ./recovery <target device or file>
This would generate the recovery disk to the <target device or file>
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
	int data_disk_num;
	int chunk_size;
	int disk_raid_type;
	int space_list_num;
	int *free_offset;	//in block number
	int *free_size;		//in number of blocks
	int *disk_size;
	char **dev_name;
	int mbr_segment_size;

	//int free_offset;
	//int free_size;
	//end ncfs
};

enum param_type {
	illegal_param, disk_total_num, data_disk_num, disk_block_size,
	    disk_raid_type, space_list_num,
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
	global_param_int(ncfs_data, line, data_disk_num, "data_disk_num");
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
			ncfs_data->disk_size =
			    (int *)malloc(sizeof(int) *
					  ncfs_data->disk_total_num);
			ncfs_data->dev_name =
			    (char **)calloc(ncfs_data->disk_total_num,
					    sizeof(char *));
		}
	}
	fclose(fp);
}

/*************************************************************************
 * Galios Field functions
 *************************************************************************/

static unsigned short *gflog;
static unsigned short *gfilog;
static const int field_power = 8;
static const unsigned int prim_poly_8 = 285;	//0x11d

/*
 * gf_gen_tables: Create logarithm and inverse logairthm tables for computing Q parity
 *
 * @param s: field size (2 power s)
 *
 * @return: 0
 */

int gf_gen_tables(int s)
{
	unsigned int b, index, gf_elements;

	//convert s to the number of elements for the table
	gf_elements = 1 << s;
	gflog = (unsigned short *)malloc(sizeof(unsigned short) * gf_elements);
	gfilog = (unsigned short *)malloc(sizeof(unsigned short) * gf_elements);
	b = 1;

	for (index = 0; index < gf_elements - 1; index++) {
		gflog[b] = (unsigned char)index;
		gfilog[index] = (unsigned char)b;
		b <<= 1;
		if (b & gf_elements) {
			b ^= prim_poly_8;
		}
	}

	return 0;
}

/*
 * gf_mul: Multiple a and b 
 *
 * @param a, b: input values for multiplication
 * @param s: field size (2 power s)
 *
 * @return: Product in finite field
 */

unsigned short gf_mul(int a, int b, int s)
{
	unsigned int field_max;
	int sum;
	unsigned short result;

	if ((a == 0) || (b == 0)) {
		result = 0;
	} else {
		field_max = (1 << s) - 1;
		sum = gflog[a] + gflog[b];
		sum = sum % field_max;

		result = gfilog[sum];
	}

	return (result);
}

/*
 * gf_div: Divide a by b
 *
 * @param a, b: input values for division
 * @param s: field size (2 power s)
 *
 * @return: Divident in finite field
 */

unsigned short gf_div(int a, int b, int s)
{
	unsigned int field_max;
	int diff;
	unsigned short result;

	if (a == 0) {
		result = 0;
	} else {
		field_max = (1 << s) - 1;

		diff = gflog[a] - gflog[b];

		if (diff < 0) {
			diff = diff + field_max;
		}

		result = gfilog[diff];
	}

	return (result);
}

/*
 * gf_get_coefficient: Get the coefficient of the primitive polynomial
 * polynomial = x^8 + x^4 + x^3 + x^2 + 1
 *
 * @param value: the coefficient offset
 * @param s: field size (2 power s)
 *
 * @return: the coefficient
 */

int gf_get_coefficient(int value, int s)
{
	int result;

	result = 1 << value;

	return result;
}

/*************************************************************************
 * MBR functions
 *************************************************************************/

/*
 * mbr_find_block_id: find block id for a block based on its disk id and block no
 *
 * @param disk_id: disk ID of the data block
 * @param block_no: block number of the data block
 * @param mbr_segment_size: the segment size for MBR coding
 *
 * @return: mbr block id
 */
int mbr_find_block_id(int disk_id, int block_no, int mbr_segment_size)
{
	int mbr_block_id;
	int block_offset, mbr_offset;
	int counter, i;

	block_offset = block_no % mbr_segment_size;

	if (block_offset < disk_id) {
		mbr_block_id = -1;
	} else {
		mbr_offset = block_offset - disk_id;
		mbr_block_id = 0;
		counter = mbr_segment_size;
		for (i = 0; i < disk_id; i++) {
			mbr_block_id = mbr_block_id + counter;
			counter--;
		}

		mbr_block_id = mbr_block_id + mbr_offset;
	}

	return mbr_block_id;
}

/*
 * mbr_find_dup_block_id: find block id for a duplicated block based on its disk id and block no
 *
 * @param disk_id: disk ID of the data block
 * @param block_no: block number of the data block
 * @param mbr_segment_size: the segment size for MBR coding
 *
 * @return: mbr block id
 */
int mbr_find_dup_block_id(int disk_id, int block_no, int mbr_segment_size)
{
	int mbr_block_id;
	int block_offset, mbr_offset;
	int counter, i;

	block_offset = block_no % mbr_segment_size;

	if (block_offset >= disk_id) {
		mbr_block_id = -1;
	} else {
		mbr_offset = disk_id - 1;
		mbr_block_id = 0;
		counter = mbr_segment_size;
		for (i = 0; i < block_offset; i++) {
			mbr_block_id = mbr_block_id + counter;
			counter--;
			mbr_offset--;
		}

		mbr_block_id = mbr_block_id + mbr_offset;
	}

	return mbr_block_id;
}

/*
 * mbr_get_disk_id: get disk id for a block based on its MBR block ID
 *
 * @param mbr_block_id: mbr block ID of the data block
 * @param mbr_segment_size: the segment size for MBR coding
 *
 * @return: disk id
 */
int mbr_get_disk_id(int mbr_block_id, int mbr_segment_size)
{
	int mbr_block_group, counter, temp;

	mbr_block_group = 0;
	counter = mbr_segment_size;
	temp = (mbr_block_id + 1) - counter;

	while (temp > 0) {
		counter--;
		temp = temp - counter;
		(mbr_block_group)++;
	}

	return mbr_block_group;
}

/*
 * mbr_get_block_no: get block number for a block based on its disk id and MBR block number
 *
 * @param disk id: disk ID of the data block
 * @param mbr_block_id: mbr block ID of the data block
 * @param mbr_segment_size: the segment size for MBR coding
 *
 * @return: block number
 */
int mbr_get_block_no(int disk_id, int mbr_block_id, int mbr_segment_size)
{
	int block_no;
	int mbr_offset, j;

	mbr_offset = mbr_block_id;
	for (j = 0; j < disk_id; j++) {
		mbr_offset = mbr_offset - (mbr_segment_size - j);
	}
	block_no = mbr_offset + disk_id;

	return block_no;
}

/*
 * mbr_get_dup_disk_id: get disk id for a duplicated block based on its MBR block ID
 *
 * @param mbr_block_id: mbr block ID of the data block
 * @param mbr_segment_size: the segment size for MBR coding
 *
 * @return: disk id
 */
int mbr_get_dup_disk_id(int mbr_block_id, int mbr_segment_size)
{
	int dup_disk_id;
	int disk_id, mbr_offset;
	int j;

	disk_id = mbr_get_disk_id(mbr_block_id, mbr_segment_size);
	mbr_offset = mbr_block_id;
	for (j = 0; j < disk_id; j++) {
		mbr_offset = mbr_offset - (mbr_segment_size - j);
	}

	dup_disk_id = mbr_offset + 1 + disk_id;

	return dup_disk_id;
}

/*
 * mbr_get_dup_block_no: get block number for a duplicated block based on its disk id and MBR block number
 *
 * @param disk id: disk ID of the original data block
 * @param mbr_block_id: mbr block ID of the data block
 * @param mbr_segment_size: the segment size for MBR coding
 *
 * @return: block number
 */
int mbr_get_dup_block_no(int disk_id, int mbr_block_id, int mbr_segment_size)
{
	int dup_block_no;

	dup_block_no = disk_id;

	return dup_block_no;
}

/*
recover_mirror: recovery for raid 1

@param ncfs_data: NCFS context data
@param fail_disk_num: failed disk number
@param fail_disk_id: list of failed disk id
@param target_dev: target new device for recovery

@return: 0 for success; -1 for error
*/
int recover_mirror(struct ncfs_state *ncfs_data, int fail_disk_num,
		   int *fail_disk_id, char *target_dev)
{
	int retstat;
	int fd_src, fd_des;
	int src_disk_id;
	int block_size, disk_size;
	int offset;
	int i;
	char *buf;

	printf("recover_mirror: fail_disk_id=%d\n", fail_disk_id[0]);
	src_disk_id = ncfs_data->disk_total_num - 1 - fail_disk_id[0];
	printf("recover_mirror: src_disk_id=%d, dev_name=%s\n", src_disk_id,
	       ncfs_data->dev_name[src_disk_id]);

	fd_src = open(ncfs_data->dev_name[src_disk_id], O_RDONLY);
	fd_des = open(target_dev, O_WRONLY);
	block_size = ncfs_data->disk_block_size;
	printf("recover_mirror: block_size=%d\n", block_size);
	buf = (char *)malloc(sizeof(char) * block_size);

	disk_size = ncfs_data->disk_size[src_disk_id];
	printf("recover_mirror: disk_size=%d\n", disk_size);
	for (i = 0; i < disk_size; i++) {
		offset = i * block_size;
		retstat = pread(fd_src, buf, block_size, offset);
		//printf("***retstat read = %d\n",retstat);
		retstat = pwrite(fd_des, buf, block_size, offset);
		//printf("***retstat write = %d\n",retstat);
	}

	close(fd_src);
	close(fd_des);
	free(buf);

	return 0;
}

/*
recover_xor: recovery for raid 4, 5

@param ncfs_data: NCFS context data
@param fail_disk_num: failed disk number
@param fail_disk_id: list of failed disk id
@param target_dev: target new device for recovery

@return: 0 for success; -1 for error
*/
int recover_xor(struct ncfs_state *ncfs_data, int fail_disk_num,
		int *fail_disk_id, char *target_dev)
{
	int retstat;
	int fd_src, fd_des;
	int src_disk_id;
	int block_size, disk_size;
	int offset;
	int i, j, k;
	char *buf, *temp_buf;

	printf("recover_xor: fail_disk_id=%d\n", fail_disk_id[0]);

	fd_des = open(target_dev, O_WRONLY);
	block_size = ncfs_data->disk_block_size;
	printf("recover_xor: block_size=%d\n", block_size);
	buf = (char *)malloc(sizeof(char) * block_size);
	temp_buf = (char *)malloc(sizeof(char) * block_size);

	disk_size = ncfs_data->disk_size[fail_disk_id[0]];
	printf("recover_xor: disk_size=%d\n", disk_size);

	for (i = 0; i < disk_size; i++) {
		offset = i * block_size;
		memset(buf, 0, block_size);
		memset(temp_buf, 0, block_size);

		for (j = 0; j < ncfs_data->disk_total_num; j++) {
			src_disk_id = j;
			if (src_disk_id != fail_disk_id[0]) {
				fd_src =
				    open(ncfs_data->dev_name[src_disk_id],
					 O_RDONLY);
				retstat =
				    pread(fd_src, temp_buf, block_size, offset);
				//printf("recover_xor: retstat read = %d\n",retstat);
				for (k = 0; k < block_size; k++) {
					buf[k] = buf[k] ^ temp_buf[k];
				}
				close(fd_src);
			}
		}

		retstat = pwrite(fd_des, buf, block_size, offset);
		//printf("***retstat write = %d\n",retstat);
	}

	close(fd_des);
	free(buf);
	free(temp_buf);

	return 0;
}

/*
recover_raid6: recovery for raid6

@param ncfs_data: NCFS context data
@param fail_disk_num: failed disk number
@param fail_disk_id: list of failed disk id
@param target_dev: target new device for recovery

@return: 0 for success; -1 for error
*/
int recover_raid6(struct ncfs_state *ncfs_data, int fail_disk_num,
		  int *fail_disk_id, char *target_dev)
{
	int retstat;
	int fd_src, fd_des;
	int src_disk_id;
	int block_size, disk_size;
	int offset;
	int i, j, k;
	char *buf, *temp_buf;

	int parity_disk_id, code_disk_id;
	int data_disk_coeff;

	printf("recover_raid6: fail_disk_id=%d\n", fail_disk_id[0]);

	fd_des = open(target_dev, O_WRONLY);
	block_size = ncfs_data->disk_block_size;
	printf("recover_raid6: block_size=%d\n", block_size);
	buf = (char *)malloc(sizeof(char) * block_size);
	temp_buf = (char *)malloc(sizeof(char) * block_size);

	disk_size = ncfs_data->disk_size[fail_disk_id[0]];
	printf("recover_raid6: disk_size=%d\n", disk_size);

	gf_gen_tables(field_power);

	for (i = 0; i < disk_size; i++) {
		offset = i * block_size;
		memset(buf, 0, block_size);
		memset(temp_buf, 0, block_size);

		code_disk_id = (ncfs_data->disk_total_num) - 1
		    - (i % (ncfs_data->disk_total_num));
		parity_disk_id = (ncfs_data->disk_total_num) - 1
		    - ((i + 1) % (ncfs_data->disk_total_num));

		if (fail_disk_id[0] != code_disk_id) {
			for (j = 0; j < ncfs_data->disk_total_num; j++) {
				src_disk_id = j;
				if ((src_disk_id != fail_disk_id[0]) &&
				    (src_disk_id != code_disk_id)) {
					fd_src =
					    open(ncfs_data->
						 dev_name[src_disk_id],
						 O_RDONLY);
					retstat =
					    pread(fd_src, temp_buf, block_size,
						  offset);
					//printf("recover_xor: retstat read = %d\n",retstat);
					for (k = 0; k < block_size; k++) {
						buf[k] = buf[k] ^ temp_buf[k];
					}
					close(fd_src);
				}
			}
		} else if (fail_disk_id[0] == code_disk_id) {
			for (j = 0; j < ncfs_data->disk_total_num; j++) {
				src_disk_id = j;
				if ((j != parity_disk_id)
				    && (j != code_disk_id)) {
					fd_src =
					    open(ncfs_data->
						 dev_name[src_disk_id],
						 O_RDONLY);
					//Cache Start
					retstat =
					    pread(fd_src, temp_buf, block_size,
						  offset);
					//Cache End

					//calculate the coefficient of the data block
					data_disk_coeff = j;

					if (j > code_disk_id) {
						(data_disk_coeff)--;
					}
					if (j > parity_disk_id) {
						(data_disk_coeff)--;
					}
					data_disk_coeff =
					    ncfs_data->disk_total_num - 3 -
					    data_disk_coeff;
					data_disk_coeff =
					    gf_get_coefficient(data_disk_coeff,
							       field_power);

					for (k = 0; k < block_size; k++) {
						//calculate code block Q
						buf[k] = buf[k] ^
						    (char)gf_mul((unsigned char)
								 temp_buf[k],
								 data_disk_coeff,
								 field_power);
					}
					close(fd_src);
				}
			}
		}

		retstat = pwrite(fd_des, buf, block_size, offset);
		//printf("***retstat write = %d\n",retstat);
	}

	close(fd_des);
	free(buf);
	free(temp_buf);

	return 0;
}

/*
recover_mbr: recovery for mbr

@param ncfs_data: NCFS context data
@param fail_disk_num: failed disk number
@param fail_disk_id: list of failed disk id
@param target_dev: target new device for recovery

@return: 0 for success; -1 for error
*/
int recover_mbr(struct ncfs_state *ncfs_data, int fail_disk_num,
		int *fail_disk_id, char *target_dev)
{
	int retstat;
	int fd_src, fd_des;
	int src_disk_id;
	int block_size, disk_size;
	int offset;
	int i, j, k, n;
	char *buf, *temp_buf;

	int mbr_segment_size;
	int mbr_segment_id;
	int mbr_block_id;
	int disk_id;
	int block_no;
	int dup_disk_id, dup_block_no;
	int flag_dup;

	printf("recover_mbr: fail_disk_id=%d\n", fail_disk_id[0]);

	fd_des = open(target_dev, O_WRONLY);
	block_size = ncfs_data->disk_block_size;
	printf("recover_mbr: block_size=%d\n", block_size);
	buf = (char *)malloc(sizeof(char) * block_size);
	temp_buf = (char *)malloc(sizeof(char) * block_size);

	disk_size = ncfs_data->disk_size[fail_disk_id[0]];
	printf("recover_mbr: disk_size=%d\n", disk_size);

	n = ncfs_data->disk_total_num;
	k = ncfs_data->data_disk_num;
	ncfs_data->mbr_segment_size =
	    (int)(2 *
		  ((k * (n - 1) - (int)(k * (k - 1) / 2)) +
		   ((int)(n * (n - 1) / 2) -
		    (int)(k * (2 * n - k - 1) / 2))) / n);
	mbr_segment_size = ncfs_data->mbr_segment_size;
	printf("recover_mbr: mbr_segment_size=%d\n", mbr_segment_size);

	disk_id = fail_disk_id[0];
	for (i = 0; i < disk_size; i++) {
		offset = i * block_size;
		memset(buf, 0, block_size);
		memset(temp_buf, 0, block_size);

		block_no = i;
		mbr_segment_id = (int)(block_no / mbr_segment_size);

		flag_dup = 0;
		mbr_block_id =
		    mbr_find_block_id(disk_id, block_no, mbr_segment_size);
		if (mbr_block_id == -1) {
			flag_dup = 1;
			mbr_block_id =
			    mbr_find_dup_block_id(disk_id, block_no,
						  mbr_segment_size);
		}

		if (flag_dup == 0) {
			dup_disk_id =
			    mbr_get_dup_disk_id(mbr_block_id, mbr_segment_size);
			dup_block_no =
			    mbr_get_dup_block_no(disk_id, mbr_block_id,
						 mbr_segment_size);
			dup_block_no =
			    dup_block_no + mbr_segment_id * mbr_segment_size;
		} else {	//if flag_dup = 1
			dup_disk_id =
			    mbr_get_disk_id(mbr_block_id, mbr_segment_size);
			dup_block_no =
			    mbr_get_block_no(dup_disk_id, mbr_block_id,
					     mbr_segment_size);
			dup_block_no =
			    dup_block_no + mbr_segment_id * mbr_segment_size;
		}

		//printf("dup_disk_id=%d, dup_block_no=%d\n",dup_disk_id,dup_block_no);

		offset = dup_block_no * block_size;
		src_disk_id = dup_disk_id;
		fd_src = open(ncfs_data->dev_name[src_disk_id], O_RDONLY);
		retstat = pread(fd_src, buf, block_size, offset);
		close(fd_src);

		offset = block_no * block_size;
		retstat = pwrite(fd_des, buf, block_size, offset);
		//printf("***retstat write = %d\n",retstat);
	}

	close(fd_des);
	free(buf);
	free(temp_buf);

	return 0;
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
		ncfs_data->disk_size[j] = ncfs_data->free_size[j];
	}

	//test print
	printf
	    ("***main: disk_total_num=%d, data_disk_num=%d, block_size=%d, raid_type=%d\n",
	     ncfs_data->disk_total_num, ncfs_data->data_disk_num,
	     ncfs_data->disk_block_size, ncfs_data->disk_raid_type);
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

	if (fail_disk_num > 0) {
		if (fail_disk_num == 1) {
			if (ncfs_data->disk_raid_type == 1) {
				printf("recover_mirror starts.\n");
				recover_mirror(ncfs_data, fail_disk_num,
					       fail_disk_id, target_dev);
			} else if ((ncfs_data->disk_raid_type == 4)
				   || (ncfs_data->disk_raid_type == 5)) {
				printf("recover_xor starts.\n");
				recover_xor(ncfs_data, fail_disk_num,
					    fail_disk_id, target_dev);
			} else if (ncfs_data->disk_raid_type == 6) {
				printf("recover_raid6 starts.\n");
				recover_raid6(ncfs_data, fail_disk_num,
					      fail_disk_id, target_dev);
			} else if (ncfs_data->disk_raid_type == 1000) {
				printf("recover_mbr starts.\n");
				recover_mbr(ncfs_data, fail_disk_num,
					    fail_disk_id, target_dev);
			} else {
				printf
				    ("Recovery of this raid type is not supported\n");
			}
		} else {
			printf("Too many disks fail.\n");
		}
	} else {
		printf("No disk fails.\n");
	}

	return 0;
}
