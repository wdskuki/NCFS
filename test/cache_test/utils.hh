#ifndef __UTILS_H_
#define __UTILS_H_

// The FUSE API has been changed a number of times.  So, our code
// needs to define the version of the API that we assume.  As of this
// writing, the most current API version is 26
#define FUSE_USE_VERSION 26

// need this to get pwrite().  I have to use setvbuf() instead of
// setlinebuf() later in consequence.
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500
#endif

//#define ECFS_DATA ((struct ecfs_state *) fuse_get_context()->private_data)

#define MAGIC_NUMBER 123
// maintain ecfs state in here
//#include <limits.h>
//#include <stdio.h>


struct ecfs_state {
    char *rootdir;

    //start ecfs
    int disk_total_num;
    int disk_block_size;
    int disk_raid_type;
    int space_list_num;
    int operation_mode; //0 for normal; 1 for degraded; 2 for incapable
    int *free_offset;   //in block number
    int *free_size;     //in number of blocks
    int *disk_status;
    char **dev_name;

    struct space_list *space_list_head;

    //end ecfs
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

struct data_block_info{
	int disk_id;
	int block_no;
};

struct raid_metadata{
	int disk_id;
	int free_offset;   //in bytes
	int free_size;   //in bytes
};


namespace filesystem
{
   class FS_manager
   {
   private:
	int parse_setting_line(struct ecfs_state *ecfs_data, char *line);
   public:
	FS_manager();	
	void ecfs_fullpath(char fpath[], const char *path);	
	int search_disk_id(const char* path);
	char* get_disk_dev_name(int disk_id);
	int round_to_block_size(int size);
	void get_raid_setting(struct ecfs_state *ecfs_data);
	void get_raid_metadata(struct ecfs_state *ecfs_data);
	void get_disk_status(struct ecfs_state *ecfs_data);
	void get_operation_mode(struct ecfs_state *ecfs_data);
   };
}//end namespace

#endif
