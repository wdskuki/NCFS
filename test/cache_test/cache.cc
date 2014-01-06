//#define _XOPEN_SOURCE 500
#include "cache.hh"
#include "utils.hh"
#include "disk.hh"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
	
static inline long long min(long long a,long long b) {
	return a < b ? a : b;
};

//ECFS context state
extern struct ecfs_state* ECFS_DATA;
extern DiskLayer* diskLayer; 

/*************************************************************************
 * Private functions
 *************************************************************************/
/*
 * LockMutex: lock mutex
 * 
 * @param id: id for file info (not used for disk cache)
 * @param CT: cache type
 */
void CacheLayer::LockMutex(int id, Cache_Type CT){
	if (CT == DISKCACHE) {
		pthread_mutex_lock(&diskcachelock_);
	} else if (CT == FILEINFOCACHE) {
		pthread_mutex_lock(&filecachelock_[id]);
	}
}

/*
 * UnlockMutex: unlock mutex
 * 
 * @param id: id for file info (not used for disk cache)
 * @param CT: cache type
 */
void CacheLayer::UnlockMutex(int id, Cache_Type CT){
	if (CT == DISKCACHE){
		pthread_mutex_unlock(&diskcachelock_);
	} else if (CT == FILEINFOCACHE){
		pthread_mutex_unlock(&filecachelock_[id]);
	}
}

/*
 * CheckBoundary: check boundary. Flush disk/fileinfo if the offset is
 * outside the boundary
 *
 * @param CE: Cache_Entry
 * @param offset: offset
 * @param CT: cache type
 */
void CacheLayer::CheckBoundary(Cache_Entry* CE, long long offset,
		Cache_Type CT) {
	if (msg_) {
		printf("CB: %d | id %d, offset %lld, start %lld, size %lld\n",
				CT, CE->id, offset, CE->startaddress, CE->contentsize);
	}
	if (log_) {
		fprintf(cachelog_, 
				"CB: %d | id %d, offset %lld, start %lld, size %lld\n", 
				CT, CE->id, offset, CE->startaddress, CE->contentsize);
	}
	if ((offset >= (CE->startaddress + CE->contentsize)) || 
			(CE->startaddress > offset)){
		if (CT == DISKCACHE)
			DiskFlush(offset);
		else if (CT == FILEINFOCACHE)
			FileInfoFlush(CE->id,offset);
	}
}


/*
 * GenericWrite: generic write operation to write buf to cache
 * 
 * @param CE: Cache_Entry
 * @param buf: data buffer to be written to cache
 * @param size: data size to be written to cache
 * @param offset: offset of where the cache starts
 * @param CT: cache type
*/
int CacheLayer::GenericWrite(Cache_Entry* CE, const char* buf, 
		long long size, long long offset,Cache_Type CT) {

	char* cacheaddress;
	const char* bufaddress;
	long long copylen;
	long long copied = 0;
	long long cacheoffset;
	int dirtystart,dirtyend,i;

	if(CE == NULL) {
		return -1;
	}

	bufaddress = buf;
	while (copied < size) {
		LockMutex(CE->id,CT);
		CheckBoundary(CE,offset + copied,CT);
		cacheoffset = offset + copied - CE->startaddress;
		cacheaddress = (char *)(CE->content) + cacheoffset;
		copylen = min((CE->contentsize - cacheoffset),(size - copied));
		memcpy(cacheaddress,bufaddress,copylen);	
		CE->dirtybit = DIRTY;
		dirtystart = cacheoffset / CE->block_size;
		dirtyend = (cacheoffset + copylen) / CE->block_size;
		if (((cacheoffset + copylen) % CE->block_size) == 0) {
			dirtyend--;
		}
		if (log_) { 
			fprintf(cachelog_,"ds %d, de %d\n",dirtystart,dirtyend);
		}
		for(i = dirtystart; i <= dirtyend; i++) {
			CE->dirtyblock[i] = DIRTY;	
		}
		bufaddress += copylen;
		copied += copylen;
		UnlockMutex(CE->id,CT);
	}
	return copied;
}


/*
 * GenericRead: generic read operation - read from cache to buffer
 * 
 * @param CE: Cache_Entry
 * @param buf: data buffer that holds results from cache
 * @param size: data size to be read from cache
 * @param offset: offset
 * @param CT: cache type
*/
int CacheLayer::GenericRead(Cache_Entry* CE, char* buf, long long size, 
		long long offset, Cache_Type CT) {
	char* cacheaddress;
	char* bufaddress;
	long long copylen;
	long long copied = 0;
	long long cacheoffset;

	if (CE == NULL) {
		return -1;
	}

	bufaddress = buf;
	while (copied < size) {
		LockMutex(CE->id,CT);
		CheckBoundary(CE,offset + copied,CT);
		cacheoffset = offset + copied - CE->startaddress;
		cacheaddress = (char *)(CE->content) + cacheoffset;
		copylen = min((CE->contentsize - cacheoffset),(size - copied));
		memcpy(bufaddress,cacheaddress,copylen);
		bufaddress += copylen;
		copied += copylen;
		UnlockMutex(CE->id,CT);
	}
	return copied;
}

/*
 * Fetch: fetch data
 * 
 * @param CE: cache entry
 * @param offset: offset of where to fetch the cache entry
 */
void CacheLayer::Fetch(Cache_Entry* CE, long long offset){
	char* dev;
	long long retstat;

	//int diskfd;
	//if(msg_)printf("+++Cache_Fetch\n");
	//Cache_Entry* CE = CA->CE;
	//disk_id = CA->disk_id;
	WriteBack(CE);
	dev = CE->name;
	//diskfd = open(dev,O_RDONLY);
	//printf("diskfd %d\n",diskfd);
	memset(CE->content,0,CACHE_SIZE);
	//retstat = pread(diskfd,CE->content,CACHE_SIZE,offset);
	retstat = diskLayer->DiskRead(CE->name, (char*)CE->content,
			CACHE_SIZE, offset);
	CE->dirtybit = CLEAN;
	memset(CE->dirtyblock,0,CACHE_SIZE / CE->block_size*sizeof(Cache_Status));
	CE->startaddress = offset;
	CE->contentsize = CACHE_SIZE;
	//close(diskfd);
	if (log_) { 
		fprintf(cachelog_,"DF: id %d, offset %lld, size %lld\n",
				CE->id,CE->startaddress,CE->contentsize);
	}
	if (msg_) { 
		printf("+++Cache_Fetch: id = %d, offset = %lld, size = %lld\n",
				CE->id,offset,retstat);
	}
} 

/*
 * WriteBack: write back
 * 
 * @param CE: Cache Entry
 */
void CacheLayer::WriteBack(Cache_Entry* CE){
	char* target;
	//int fd;
	int retstat;
	int i,j;
	int dirtystart = -1;
	char* contentaddress;

	if(CE->dirtybit == DIRTY){
		target = CE->name;
		//fd = open(target,O_WRONLY);

		for(i = 0; i < CE->contentsize / CE->block_size; i++){
			if (CE->dirtyblock[i] == DIRTY){
				if(dirtystart == -1)
					dirtystart = i;
				CE->dirtyblock[i] = PROCESSING;
			} else {
				if(dirtystart != -1){
					contentaddress = (char*)(CE->content) + 
						(CE->block_size * dirtystart);
					//retstat = pwrite(fd,contentaddress,(i - dirtystart) * CA->block_size,CA->startaddress + (CA->block_size * dirtystart));
					retstat = diskLayer->DiskWrite(CE->name, contentaddress,
							(i - dirtystart) *
							CE->block_size,CE->startaddress +
							(CE->block_size * dirtystart));
					if (log_) { 
						fprintf(cachelog_,
								"WB: %s, start %d, end %d, ret %d\n",
								CE->name,dirtystart,(i-1),retstat);
					}
					dirtystart = -1;
					for (j = dirtystart; j < i; j++){
						CE->dirtyblock[j] = CLEAN;
					}
				}
			}
		}
		i = CE->contentsize / CE->block_size;
		if (dirtystart != -1){
			contentaddress = (char *)(CE->content) + 
				(CE->block_size * dirtystart);
			//retstat = pwrite(fd,contentaddress,(i - dirtystart) * CE->block_size,CE->startaddress + (CE->block_size * dirtystart));
			retstat = diskLayer->DiskWrite(CE->name,contentaddress,
					(i - dirtystart) * CE->block_size, 
					CE->startaddress + (CE->block_size * dirtystart));
			if (log_) { 
				fprintf(cachelog_,
						"WB: %s, start %d, end %d, ret %d\n",
						CE->name,dirtystart,(i-1),retstat);
			}
			dirtystart = -1;
			for (j = dirtystart; j < i; j++){
				CE->dirtyblock[j] = CLEAN;
			}
		}

		//close(fd);
		CE->dirtybit = CLEAN;
	}
}

/*****************************************************************************
 * Public functions
 *****************************************************************************/
/*
 * Constructor - create disk cache and file info cache
 *
 * @param ecfs_state: ECFS context state (with disk information)
 */
CacheLayer::CacheLayer(struct ecfs_state* ecfs_data) {

	int block_size;
	int i;

	// XXX: pclee - should we allocate memory for a maximum number of
	// possible disks and dynamically associate new disk?
	
	// create the cache log 	
	cachelog_ = fopen("cachelog","w+");
	
	// create the disk cache based on the number of disks we have
	numofdisk_ = ecfs_data->disk_total_num;    // set the number of disks
	block_size = ecfs_data->disk_block_size;
	diskcache_ = (Cache_Entry**)malloc(sizeof(Cache_Entry*) * numofdisk_);
	for(i = 0; i < numofdisk_; i++){
		diskcache_[i] = (Cache_Entry*)malloc(sizeof(Cache_Entry));
		diskcache_[i]->content = (void*)malloc(CACHE_SIZE);
		diskcache_[i]->dirtybit = CLEAN;
		diskcache_[i]->dirtyblock = (Cache_Status*)
			calloc(CACHE_SIZE / block_size, sizeof(Cache_Status));
		diskcache_[i]->block_size = block_size;
		diskcache_[i]->startaddress = -1;
		diskcache_[i]->contentsize = 0;
		strncpy(diskcache_[i]->name, ecfs_data->dev_name[i], 32);
		diskcache_[i]->id = i;
		if (msg_) {
			printf("+++Cache_DiskCreate disk = %d\n",i);
		}
		if (log_) { 
			fprintf(cachelog_,"+++Cache_DiskCreate disk = %d\n",i);
		}
	}
	
	// create the file info cache (do not initiate name at this point)
	for (i=0; i<FILE_MAX; ++i) {
		fileinfocache_[i] = (Cache_Entry*)malloc(sizeof(Cache_Entry));
		fileinfocache_[i]->content = (void*)malloc(CACHE_SIZE);
		fileinfocache_[i]->dirtybit = CLEAN;
		fileinfocache_[i]->dirtyblock = (Cache_Status*)
			calloc(CACHE_SIZE / sizeof(struct data_block_info), 
					sizeof(Cache_Status));
		fileinfocache_[i]->block_size = sizeof(struct data_block_info);
		fileinfocache_[i]->startaddress = -1;
		fileinfocache_[i]->contentsize = 0;
		// fileinfocache_[id]->name (leave it alone until open)
		fileinfocache_[i]->id = 0;
	}

	// init cache locks
	pthread_mutex_init(&diskcachelock_, NULL);
	for (i=0; i<FILE_MAX; ++i) {	
		pthread_mutex_init(&filecachelock_[i],NULL);
	}
}

/*
 * DiskWrite: write data to disk
 * 
 * @param disk_id: id
 * @param buf: data buffer
 * @param size: data size
 * @param offset: offset
*/
int CacheLayer::DiskWrite(int disk_id, const char* buf, long long size, 
		long long offset){

	if (msg_) { 
		printf("+++Cache_DiskWrite: disk = %d, buf = %ld, size = %lld, offset = %lld\n",disk_id,(long int)buf,size,offset);
	}
	if (log_) {
		fprintf(cachelog_,"DW: disk %d, size %lld, offset %lld\n",
				disk_id,size,offset);
	}
	return GenericWrite(diskcache_[disk_id],buf,size,offset,DISKCACHE);
}


/*
 * DiskRead: read data from disk
 * 
 * @param disk_id: id
 * @param buf: data buffer
 * @param size: data size
 * @param offset: offset
*/
int CacheLayer::DiskRead(int disk_id, char* buf, long long size, 
		long long offset){
	if (msg_) { 
		printf("+++Cache_DiskRead: disk = %d, buf = %ld, size = %lld, offset = %lld\n",disk_id,(long int)buf,size,offset);
	}
	if (log_) { 
		fprintf(cachelog_,"DR: disk %d, size %lld, offset %lld\n",
				disk_id,size,offset);
	}
	return GenericRead(diskcache_[disk_id],buf,size,offset,DISKCACHE);
}


/*
 * DiskFlush: flush data to disk
 * 
 * @param offset: offset
 */
void CacheLayer::DiskFlush(long long offset){
	long long retstat;
	int i;
	//Cache_Argument CA[numofdisk_];

	if (log_) {
		fprintf(cachelog_,"+++Cache_DiskFlush: offset = %lld\n",offset);
	}
	for(i = 0; i < numofdisk_; i++){
		//int diskfd;
		//if(msg_)printf("+++Cache_Fetch\n");          
		Cache_Entry* CE = diskcache_[i]; 
		//disk_id = CA->disk_id;
		WriteBack(CE);
//		Fetch(CE);
		// offset = CA->offset;
		//dev = CE->name;
		//diskfd = open(dev,O_RDONLY);
		//printf("diskfd %d\n",diskfd);
		memset(CE->content,0,CACHE_SIZE);
		//retstat = pread(diskfd,CE->content,CACHE_SIZE,offset);
		retstat = diskLayer->DiskRead(CE->name, (char*)CE->content,
				CACHE_SIZE, offset);
		CE->dirtybit = CLEAN;
		memset(CE->dirtyblock,0,CACHE_SIZE / CE->block_size*sizeof(Cache_Status));
		CE->startaddress = offset;
		CE->contentsize = CACHE_SIZE;
		//close(diskfd);
		if (log_) {
			fprintf(cachelog_,"DF: id %d, offset %lld, size %lld\n",
					CE->id,CE->startaddress,CE->contentsize);
		}
		if (msg_) { 
			printf("+++Cache_Fetch: id = %d, offset = %lld, size = %lld\n",
					CE->id,offset,retstat);
		}
	}
}

/*
 * DiskDestroy: destroy disk cache
 */
void CacheLayer::DiskDestroy(void){
	int i;
	if (msg_) {
		printf("+++Cache_DiskDestroy\n");
	}
	if (log_) { 
		fprintf(cachelog_,"+++Cache_DiskDestroy\n");
	}
	for(i = 0; i < numofdisk_; i++){
		WriteBack(diskcache_[i]);
	}
	return;
}

/*****
 * File Info Cache
 *****/

/*
 * Cache_FileInfoWrite: write file info to cache
 * 
 * @param id: id
 * @param buf: data buffer
 * @param size: size
 * @param offset: offset
 * @return: size of successful write
*/
int CacheLayer::FileInfoWrite(int id, const char* buf, long long size, 
		long long offset){
	if (msg_) { 
		printf("+++Cache_FileInfoWrite: id %d, size %lld, offset %lld\n",
				id,size,offset);
	}
	if (log_) { 
		fprintf(cachelog_,"FW: id %d, size %lld, offset %lld\n",id,size,offset);
	}
	if(id > FILE_MAX){
		if (log_) {
			fprintf(cachelog_,"FW: id %d exceeed\n\r",id);
		}
		return -1;
	} else {
		return GenericWrite(fileinfocache_[id],buf,size,offset,FILEINFOCACHE);
	}
}

/*
 * FileInfoRead: read file info from cache
 * 
 * @param id: id
 * @param buf: data buffer
 * @param size: size
 * @param offset offset
 * @return: size of successful read
 */
int CacheLayer::FileInfoRead(int id, char* buf, long long size, 
		long long offset) {

	if (msg_) { 
		printf("+++Cache_FileInfoRead: id %d, size %lld, offset %lld\n",
				id,size,offset);
	}
	if (log_) { 
		fprintf(cachelog_,"FR: id %d, size %lld, offset %lld\n",id,size,offset);
	}
	if (id > FILE_MAX){
		if (log_) {
			fprintf(cachelog_,"FR: id %d exceeed\n\r",id);
		}
		return -1;
	} else {
		return GenericRead(fileinfocache_[id],(char*)buf,size,offset,
				FILEINFOCACHE);
	}
}


/*
 * FileInfoFlush: flush file info to file
 * 
 * @param id: id
 * @param offset: offset
 */
void CacheLayer::FileInfoFlush(int id, long long offset){
	if (msg_) { 
		printf("+++Cache_FileInfoFlush: id %d, offset %lld\n",id,offset);
	}
	if (log_) { 
		fprintf(cachelog_,"FF: id = %d, offset = %lld\n",id,offset);
	}
	Fetch(fileinfocache_[id], offset); 
}

/*
 * FileInfoCreate: create file info cache
 * 
 * @param path: path
 * @param id: id
 */
void CacheLayer::FileInfoCreate(const char* path, int id){
	if (msg_) { 
		printf("+++Cache_FileInfoCreate: %s, id %d\n",path,id);
	}
	if (log_) { 
		fprintf(cachelog_,"FC: path %s, id %d\n",path,id);
	}
	strncpy(fileinfocache_[id]->name, path, 32);
	fileinfocache_[id]->id = id;
}

/*
 * FileInfoDestroy: destroy file info cache
 * 
 * @param id: id
 */
void CacheLayer::FileInfoDestroy(int id){
	if (msg_) { 
		printf("+++Cache_FileInfoDestroy: id %d\n",id);
	}
	if (log_) { 
		fprintf(cachelog_,"+++Cache_FileInfoDestroy: id %d\n",id);
	}
	if (fileinfocache_[id] != NULL){
		WriteBack(fileinfocache_[id]);
		pthread_mutex_destroy(&filecachelock_[id]);
	}
	return ;
}


/*
 * FileInfoSearch: search file info
 * 
 * @param path: file path
 * @return: i >= 0 for success; -1 for error
 */
int CacheLayer::FileInfoSearch(const char* path){
	int i;
	for(i = 0; i < FILE_MAX; ++i){
		if(fileinfocache_[i] != NULL){
			if (strncmp(path, fileinfocache_[i]->name, 
						sizeof(fileinfocache_[i]->name)) == 0)
				return i;
		}
	}
	return -1;
}

/*
void* Cache_Fetch_Thread(void* data){               
        long long offset;
        char* dev;
        long long retstat;

	Disk_manager disk_man;

	//int diskfd;
	//if(msg_)printf("+++Cache_Fetch\n");          
	Cache_Argument* CA = (Cache_Argument*)data; 
	Cache_Entry* CE = CA->CE;
	//disk_id = CA->disk_id;
	cache_man.Cache_WriteBack(CE);                
	offset = CA->offset;
	dev = CE->name;
	//diskfd = open(dev,O_RDONLY);
	//printf("diskfd %d\n",diskfd);
	memset(CE->content,0,CACHE_SIZE);
	//retstat = pread(diskfd,CE->content,CACHE_SIZE,offset);
	retstat = disk_man.DiskRead(CE->name,(char*)CE->content,CACHE_SIZE,offset);
	CE->dirtybit = CLEAN;
	memset(CE->dirtyblock,0,CACHE_SIZE / CE->block_size*sizeof(Cache_Status));
	CE->startaddress = offset;
	CE->contentsize = CACHE_SIZE;
	//close(diskfd);
	if (log_) {
		fprintf(cachelog_,"DF: id %d, offset %lld, size %lld\n",
				CE->id,CE->startaddress,CE->contentsize);
	}
	if (msg_) { 
		printf("+++Cache_Fetch: id = %d, offset = %lld, size = %lld\n",
				CE->id,offset,retstat);
	}
} 
*/
