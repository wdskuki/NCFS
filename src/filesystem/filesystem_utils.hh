#ifndef __FILESYSTEM_UTILS_H_
#define __FILESYSTEM_UTILS_H_

#include "../coding/coding.hh"

class FileSystemLayer {
 private:
	int parse_setting_line(struct ncfs_state *ncfs_data, char *line);

 public:
	//object of Storage coding layer
	 CodingLayer * codingLayer;

	 FileSystemLayer();
	void ncfs_fullpath(char fpath[], const char *path);
	void ncfs_mountpath(char fpath[], const char *path);
	char *get_disk_dev_name(int disk_id);
	int round_to_block_size(int size);
	void get_raid_setting(struct ncfs_state *ncfs_data);  //READ from raid_setting
	void get_raid_metadata(struct ncfs_state *ncfs_data); //READ from raid_metadata
	void get_disk_status(struct ncfs_state *ncfs_data); //WRITE TO raid_health
	void get_operation_mode(struct ncfs_state *ncfs_data); //get opt_mode from ncfs_data, and write back to ncfs_data
	int space_list_add(int disk_id, int disk_block_no); //affect NCFS_DATA
	int space_list_remove(int disk_id, int disk_block_no); //affect NCFS_DATA
	void process_command(void);
	void print_device_setting(void);
	void set_device_status(int diskid, int status);
	int get_fail_num(void);
	void update_settting(void); //write back to raid_setting
	
	//Add by zhuyunfeng on October 11, 2011 begin.
	void readSystemConfig(struct ncfs_state* ncfs_data); //read from xml file
	//Add by zhuyunfeng on October 11, 2011 end.

	//Add by Dongsheng Wei on Jan. 10, 2014 begin.
	void print_device_setting_withInput(ncfs_state* );
	//Add by Dongsheng Wei on Jan. 10, 2014 begin.
};

#endif
