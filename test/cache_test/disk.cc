#include "disk.hh"
#include "utils.hh"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extern struct ecfs_state* ECFS_DATA;

using namespace filesystem;

/*
 * Constructor
 */
DiskLayer::DiskLayer() {
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
int DiskLayer::DiskRead(const char* path, char* buf, long long size, 
		long long offset){
	int fd = open(path,O_RDONLY);
	FS_manager fs_man;

	if(fd < 0){
		int id = fs_man.search_disk_id(path);
		ECFS_DATA->disk_status[id] = 1;
		printf("///DiskRead %s failed\n",path);
		return fd;
	}
	int retstat = pread(fd,buf,size,offset);
	if(retstat < 0){
		int id = fs_man.search_disk_id(path);
		ECFS_DATA->disk_status[id] = 1;
		printf("///DiskRead %s failed\n",path);
		close(fd);
		return retstat;
	}
	close(fd);
	printf("///DiskRead path %s, size %lld, offset %lld, retstat %d\n",path,size,offset,retstat);
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
int DiskLayer::DiskWrite(const char* path, const char* buf, long long size, 
		long long offset){
	int fd = open(path,O_WRONLY);
	FS_manager fs_man;
	
	if(fd < 0){
		int id = fs_man.search_disk_id(path);
		ECFS_DATA->disk_status[id] = 1;
		printf("///DiskWrite %s failed\n",path);
		return fd;
	}
	int retstat = pwrite(fd,buf,size,offset);
	if(retstat < 0){
		int id = fs_man.search_disk_id(path);
		ECFS_DATA->disk_status[id] = 1;
		printf("///DiskWrite %s failed\n",path);
		close(fd);
		return retstat;
	}
	close(fd);
	printf("///DiskWrite path %s, size %lld, offset %lld, retstat %d\n",
			path,size,offset,retstat);
	return retstat;
}


/*
 * space_list_add: Add (deleted) space node to space list
 * 
 * @param disk_id: disk id
 * @param disk_block_no: disk block number
 * @return: space list number
 */
int DiskLayer::space_list_add(int disk_id, int disk_block_no) 
{
	int space_list_num;
	int found;
	struct space_list *new_node;
	struct space_list *current, *prev;

	space_list_num = ECFS_DATA->space_list_num;
	
	new_node = (struct space_list*)malloc(sizeof(struct space_list));
	new_node->disk_id = disk_id;
	new_node->disk_block_no = disk_block_no;
	new_node->next = NULL;

	printf("***space_list_add0: disk_id=%d, disk_block_no=%d, space_list_num=%d\n"
			,disk_id,disk_block_no,space_list_num);

	if (ECFS_DATA->space_list_head == NULL){
		printf("***space_list_add1: first node\n");
		ECFS_DATA->space_list_head = new_node;
		space_list_num = 1;
	}
	else{
		printf("***space_list_add2: list not null\n");
		current = ECFS_DATA->space_list_head;
		prev = ECFS_DATA->space_list_head;
		found = 0;
		while ((current != NULL) && (found == 0)){
			if ((disk_id <= (current->disk_id)) && 
					(disk_block_no < (current->disk_block_no))){
				found = 1;
			}
			else{
				prev = current;
				current = current->next;
			}
		}

		if (prev == current){	//list head
			new_node->next = ECFS_DATA->space_list_head;
			ECFS_DATA->space_list_head = new_node;
		}
		else{	
			new_node->next = current;	
			prev->next = new_node;
		}

		space_list_num++;

		printf("***space_list_add2: space_list_num=%d\n",space_list_num);
	}

	ECFS_DATA->space_list_num = space_list_num;
	
	return space_list_num;
}

/*
 * space_list_remove: Remove space node from space list
 * 
 * @param disk_id: disk id
 * @param disk_block_no: disk block number
 * @return: 0 for success; -1 for error
 */
int DiskLayer::space_list_remove(int disk_id, int disk_block_no) 
{
	int found, retstat;	
	struct space_list *current, *prev;

	current = ECFS_DATA->space_list_head;
	prev = ECFS_DATA->space_list_head;

	printf("***space_list_remove0: disk_id=%d, disk_block_no=%d\n",
							disk_id,disk_block_no);

	found = 0;
	if (current != NULL){
		while ((current != NULL) && (found == 0)){
			if ((disk_id == (current->disk_id)) && 
			(disk_block_no == (current->disk_block_no))){
				found = 1;
			}
			else{
				prev = current;
				current = current->next;
			}
		}

		if (found == 1){
			if (prev == current){ //list head
				ECFS_DATA->space_list_head = current->next;
			}
			else{
				prev->next = current->next;
			}
			free(current);
			(ECFS_DATA->space_list_num)--;
		}
	}

	printf("***space_list_remove3: found=%d, space_list_num=%d\n",
				found,ECFS_DATA->space_list_num);
	
	if (found == 0){
		retstat = -1;
	}
	else{
		retstat = 0;
	}

	return retstat;
}
