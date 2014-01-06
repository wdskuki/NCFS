#ifndef __FILESYSTEM_COMMON_H_
#define __FILESYSTEM_COMMON_H_

// The FUSE API has been changed a number of times.  So, our code
// needs to define the version of the API that we assume.  As of this
// writing, the most current API version is 26
#define FUSE_USE_VERSION 26

// need this to get pwrite().  I have to use setvbuf() instead of
// setlinebuf() later in consequence.
//#define _XOPEN_SOURCE 500

//#define NCFS_DATA ((struct ncfs_state *) fuse_get_context()->private_data)

#define MAGIC_NUMBER 123
// maintain ncfs state in here
//#include <limits.h>
//#include <stdio.h>

struct ncfs_state {
	char *rootdir;
	char *mountdir;

	//start ncfs
	int disk_total_num;
	int data_disk_num;
	int chunk_size;
	int disk_raid_type;
	int space_list_num;
	int operation_mode;	//0 for normal; 1 for degraded; 2 for incapable
	int *disk_size;		//disk maximum size in number of blocks
	int *free_offset;	//in block number
	int *free_size;		//in number of blocks
	int *disk_status;
	char **dev_name;

	struct space_list *space_list_head;
	int no_cache;		//1 for no cache
	int no_gui;		//1 for no gui
	int run_experiment;	//1 for running experiment

	int process_state;	//0 for normal, 1 for recovery
	double encoding_time;	//in usec
	double decoding_time;	//in usec
	double diskread_time;	//in usec
	double diskwrite_time;	//in usec

	int segment_size;

	int mbr_n;
	int mbr_k;
	int mbr_m;
	int mbr_c;
	int mbr_segment_size;

	// (m*w) * (k*w). m is fail tolerance and k is data disk number
	int *generator_matrix;
	//end ncfs
	
	//for self-defined block size
	int usebigblock;
    
};

struct space_info {
	int disk_id;
	int disk_block_no;	// in blocks
};

struct space_list {
	int disk_id;
	int disk_block_no;	// in blocks
	struct space_list *next;
};

struct data_block_info {
	int disk_id;
	int block_no;
};

struct raid_metadata {
	int disk_id;
	int free_offset;	//in bytes
	int free_size;		//in bytes
};

#endif
