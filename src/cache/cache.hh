/*
 * cache.hh
 */
#ifndef __CACHE_HH__
#define __CACHE_HH__

#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <stdlib.h>
#include <limits.h>
#include "../filesystem/filesystem_common.hh"

//#define CACHE_SIZE    8*1024*1024
#define CACHE_SIZE	5*1024*1024
#define FILE_MAX	1024
#define MultiThread

// forward declaration
//struct ncfs_state; 

void *Cache_Fetch_Thread(void *data);

class CacheLayer {
 private:
	// type definition
	typedef enum { CLEAN = 0, DIRTY, PROCESSING, DECODED } Cache_Status;
	typedef enum { DISKCACHE = 0, FILEINFOCACHE } Cache_Type;

	typedef struct {
		void *content;
		Cache_Status dirtybit;
		Cache_Status *dirtyblock;
		int block_size;
		long long startaddress;
		long long contentsize;
		char name[PATH_MAX];	// device/file name (assume 32 byte max)
		int id;
	} Cache_Entry;

	// constants
	static const bool msg_ = false;
	static const bool log_ = false;

	// data members
	Cache_Entry **diskcache_;	// disk cache
	Cache_Entry *fileinfocache_[FILE_MAX];	// file info cache
	int numofdisk_;		// number of disks maintained by cache
	FILE *cachelog_;	// cache log
	pthread_mutex_t diskcachelock_;
	pthread_mutex_t filecachelock_[FILE_MAX];

	// private functions
	void LockMutex(int id, Cache_Type CT);
	void UnlockMutex(int id, Cache_Type CT);
	void CheckBoundary(Cache_Entry * CE, long long offset, Cache_Type CT);
	int GenericWrite(Cache_Entry * CE, const char *buf, long long size,
			 long long offset, Cache_Type CT);
	int GenericRead(Cache_Entry * CE, char *buf, long long size,
			long long offset, Cache_Type CT);
	void Fetch(Cache_Entry * CE, long long offset);
	void WriteBack(Cache_Entry * CE);
	void DiskFlush(long long offset);
	void FileInfoFlush(int id, long long offset);

 public:
	// public functions
	 CacheLayer(struct ncfs_state *ncfs_data);
	int DiskWrite(int disk_id, const char *buf, long long size,
		      long long offset);
	int DiskRead(int disk_id, char *buf, long long size, long long offset);
	void DiskDestroy(void);
	int FileInfoRead(int id, char *buf, long long size, long long offset);
	int FileInfoWrite(int id, const char *buf, long long size,
			  long long offset);
	void FileInfoCreate(const char *path, int id);
	int FileInfoSearch(const char *path);
	void FileInfoDestroy(int id);
	void DiskCacheName(int diskid, char *newdevice);
	typedef struct {
		Cache_Entry *CE;
		long long offset;
		CacheLayer *This;
	} Cache_Argument;
	static void *Fetch_Thread_Func(void *data);
};

#endif
