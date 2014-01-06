#ifndef __STORAGE_HH__
#define __STORAGE_HH__

class StorageLayer {
 private:
	int search_disk_id(const char *path);
 public:
	 StorageLayer();
	
	int DiskRead(const char *path, char *buf, long long size,
		     long long offset);
	int DiskWrite(const char *path, const char *buf, long long size,
		      long long offset);
};

#endif
