#ifndef __DISK_HH__
#define __DISK_HH__

class DiskLayer {
public:
	DiskLayer();
	int DiskRead(const char* path, char* buf, long long size, long long offset);
	int DiskWrite(const char* path, const char* buf, long long size, 
			long long offset);
	int space_list_add(int disk_id, int disk_block_no);
	int space_list_remove(int disk_id, int disk_block_no);
};

#endif
