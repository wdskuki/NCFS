//#define _XOPEN_SOURCE 500
#include "cache.hh"
#include "../storage/storage.hh"
#include "../gui/process_report.hh"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

static inline long long min(long long a, long long b)
{
	return a < b ? a : b;
};

//NCFS context state
extern struct ncfs_state *NCFS_DATA;
extern StorageLayer *storageLayer;
extern ProcessReport *processReport;

/*************************************************************************
 * Private functions
 *************************************************************************/
/*
 * LockMutex: lock mutex
 * 
 * @param id: id for file info (not used for disk cache)
 * @param CT: cache type
 */
void CacheLayer::LockMutex(int id, Cache_Type CT)
{
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
void CacheLayer::UnlockMutex(int id, Cache_Type CT)
{
	if (CT == DISKCACHE) {
		pthread_mutex_unlock(&diskcachelock_);
	} else if (CT == FILEINFOCACHE) {
		pthread_mutex_unlock(&filecachelock_[id]);
	}
}

/*
 * CheckBoundary: check boundary. Flush disk/fileinfo if the offset is
 * outside the boundary
 *
 * @param CE: Cache_Entry
 * @param offset: cache offset, where to write data in cache
 * @param CT: cache type
 */
void CacheLayer::CheckBoundary(Cache_Entry * CE, long long offset,
			       Cache_Type CT)
{
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
	    (CE->startaddress > offset)) {
		if (CT == DISKCACHE)
			DiskFlush(offset);
		else if (CT == FILEINFOCACHE)
			FileInfoFlush(CE->id, offset);
	}
}

/*
 * GenericWrite: generic write operation to write buf to cache
 * 
 * @param CE: Cache_Entry
 * @param buf: data buffer to be written to cache
 * @param size: data size to be written to cache
 * @param offset: offset on disk/file, where caching starts
 * @param CT: cache type
 */
int CacheLayer::GenericWrite(Cache_Entry * CE, const char *buf,
			     long long size, long long offset, Cache_Type CT)
{
	//      new ProcessItem(CACHE2FILE,true);
	if (NCFS_DATA->no_gui == 0) {
		processReport->Update(CACHE2FILE, true);
	}

	char *cacheaddress;
	const char *bufaddress;
	long long copylen;
	long long copied = 0;
	long long cacheoffset;
	int dirtystart, dirtyend, i;

	if (CE == NULL) {
		return -1;
	}

	bufaddress = buf;
	while (copied < size) {
		LockMutex(CE->id, CT);
		CheckBoundary(CE, offset + copied, CT);
		cacheoffset = offset + copied - CE->startaddress;
		cacheaddress = (char *)(CE->content) + cacheoffset;
		copylen = min((CE->contentsize - cacheoffset), (size - copied));
		memcpy(cacheaddress, bufaddress, copylen);
		CE->dirtybit = DIRTY;
		dirtystart = cacheoffset / CE->block_size;
		dirtyend = (cacheoffset + copylen) / CE->block_size;
		if (((cacheoffset + copylen) % CE->block_size) == 0) {
			dirtyend--;
		}
		if (log_) {
			fprintf(cachelog_, "ds %d, de %d\n", dirtystart,
				dirtyend);
		}
		for (i = dirtystart; i <= dirtyend; i++) {
			CE->dirtyblock[i] = DIRTY;
		}
		bufaddress += copylen;
		copied += copylen;
		UnlockMutex(CE->id, CT);
	}
	if (NCFS_DATA->no_gui == 0) {
		processReport->Update(CACHE2FILE, false);
	}
	//      new ProcessItem(CACHE2FILE,false);
	return copied;
}

/*
 * GenericRead: generic read operation - read from cache to buffer
 * 
 * @param CE: Cache_Entry
 * @param buf: data buffer that holds results from cache
 * @param size: data size to be read from cache
 * @param offset: offset of disk/file, where caching starts
 * @param CT: cache type
 */
int CacheLayer::GenericRead(Cache_Entry * CE, char *buf, long long size,
			    long long offset, Cache_Type CT)
{
	//      new ProcessItem(CACHE2FILE,true);
	if (NCFS_DATA->no_gui == 0) {
		processReport->Update(CACHE2FILE, true);
	}
	char *cacheaddress;
	char *bufaddress;
	long long copylen;
	long long copied = 0;
	long long cacheoffset;

	if (CE == NULL) {
		return -1;
	}
	if (msg_)
		fprintf(stderr, "%s %d\n", __func__, __LINE__);

	bufaddress = buf;
	while (copied < size) {
		LockMutex(CE->id, CT);
		CheckBoundary(CE, offset + copied, CT);
		cacheoffset = offset + copied - CE->startaddress;
		cacheaddress = (char *)(CE->content) + cacheoffset;
		copylen = min((CE->contentsize - cacheoffset), (size - copied));
		memcpy(bufaddress, cacheaddress, copylen);
		bufaddress += copylen;
		copied += copylen;
		UnlockMutex(CE->id, CT);
	}
	//      new ProcessItem(CACHE2FILE,false);
	if (NCFS_DATA->no_gui == 0) {
		processReport->Update(CACHE2FILE, false);
	}
	return copied;
}

/*
 * Fetch: fetch data
 * 
 * @param CE: cache entry
 * @param offset: offset of disk/file, where to fetch the data
 */
void CacheLayer::Fetch(Cache_Entry * CE, long long offset)
{
	char *dev;
	long long retstat;
	long long startaddress = offset;
	long long readsize = CACHE_SIZE;

	struct timeval t1, t2;
	double duration;

	startaddress -=
	    offset % (NCFS_DATA->segment_size * NCFS_DATA->chunk_size);
	readsize -=
	    CACHE_SIZE % (NCFS_DATA->segment_size * NCFS_DATA->chunk_size);

	//int diskfd;
	//if(msg_)printf("+++Cache_Fetch\n");
	//Cache_Entry* CE = CA->CE;
	//disk_id = CA->disk_id;

	WriteBack(CE);

	dev = CE->name;
	//diskfd = open(dev,O_RDONLY);
	//printf("diskfd %d\n",diskfd);
	memset(CE->content, 0, CACHE_SIZE);
	//retstat = pread(diskfd,CE->content,CACHE_SIZE,offset);

	//Added by michael, to disable read from failed disk during recovery

	if (NCFS_DATA->run_experiment == 1) {
		gettimeofday(&t1, NULL);
	}

	//if (NCFS_DATA->disk_status[CE->id] == 0){
	retstat = storageLayer->DiskRead(CE->name, (char *)CE->content,
					 readsize, startaddress);
	//}

	if (NCFS_DATA->run_experiment == 1) {
		gettimeofday(&t2, NULL);

		duration = (t2.tv_sec - t1.tv_sec) +
		    (t2.tv_usec - t1.tv_usec) / 1000000.0;
		NCFS_DATA->diskread_time += duration;
	}

	CE->dirtybit = CLEAN;
	memset(CE->dirtyblock, 0,
	       CACHE_SIZE / CE->block_size * sizeof(Cache_Status));
	CE->startaddress = startaddress;
	CE->contentsize = readsize;
	//close(diskfd);
	if (log_) {
		fprintf(cachelog_, "DF: id %d, offset %lld, size %lld\n",
			CE->id, CE->startaddress, CE->contentsize);
	}
	if (msg_) {
		printf("+++Cache_Fetch: id = %d, offset = %lld, size = %lld\n",
		       CE->id, offset, retstat);
	}

	if (NCFS_DATA->run_experiment == 1) {
		fprintf(cachelog_, "*****time counters*****\n");
		fprintf(cachelog_, "Encoding time: %lf\n",
			NCFS_DATA->encoding_time);
		fprintf(cachelog_, "Decoding time: %lf\n",
			NCFS_DATA->decoding_time);
		fprintf(cachelog_, "Disk Read time: %lf\n",
			NCFS_DATA->diskread_time);
		fprintf(cachelog_, "Disk Write time: %lf\n",
			NCFS_DATA->diskwrite_time);
		fprintf(cachelog_, "**********\n");
	}
}

/*
 * WriteBack: write back
 * 
 * @param CE: Cache Entry
 */
void CacheLayer::WriteBack(Cache_Entry * CE)
{
	char *target;
	//int fd;
	int retstat;
	int i, j;
	int dirtystart = -1;
	char *contentaddress;

	struct timeval t1, t2;
	double duration;

	if (CE->dirtybit == DIRTY) {
		target = CE->name;
		//fd = open(target,O_WRONLY);

		for (i = 0; i < CE->contentsize / CE->block_size; i++) {
			if (CE->dirtyblock[i] == DIRTY) {
				if (dirtystart == -1)
					dirtystart = i;
				CE->dirtyblock[i] = PROCESSING;
			} else {
				if (dirtystart != -1) {
					contentaddress = (char *)(CE->content) +
					    (CE->block_size * dirtystart);
					//retstat = pwrite(fd,contentaddress,(i - dirtystart) * CA->block_size,CA->startaddress + (CA->block_size * dirtystart));

					if (NCFS_DATA->run_experiment == 1) {
						gettimeofday(&t1, NULL);
					}

					retstat =
					    storageLayer->DiskWrite(CE->name,
								    contentaddress,
								    (i -
								     dirtystart)
								    *
								    CE->
								    block_size,
								    CE->
								    startaddress
								    +
								    (CE->
								     block_size
								     *
								     dirtystart));

					if (NCFS_DATA->run_experiment == 1) {
						gettimeofday(&t2, NULL);

						duration =
						    (t2.tv_sec - t1.tv_sec) +
						    (t2.tv_usec -
						     t1.tv_usec) / 1000000.0;
						NCFS_DATA->diskwrite_time +=
						    duration;
					}

					if (log_) {
						fprintf(cachelog_,
							"WB: %s, start %d, end %d, ret %d\n",
							CE->name, dirtystart,
							(i - 1), retstat);
					}
					dirtystart = -1;
					for (j = dirtystart; j < i; j++) {
						CE->dirtyblock[j] = CLEAN;
					}
				}
			}
		}
		i = CE->contentsize / CE->block_size;
		if (dirtystart != -1) {
			contentaddress = (char *)(CE->content) +
			    (CE->block_size * dirtystart);

			if (NCFS_DATA->run_experiment == 1) {
				gettimeofday(&t1, NULL);
			}
			//retstat = pwrite(fd,contentaddress,(i - dirtystart) * CE->block_size,CE->startaddress + (CE->block_size * dirtystart));
			retstat =
			    storageLayer->DiskWrite(CE->name, contentaddress,
						    (i -
						     dirtystart) *
						    CE->block_size,
						    CE->startaddress +
						    (CE->block_size *
						     dirtystart));

			if (NCFS_DATA->run_experiment == 1) {
				gettimeofday(&t2, NULL);

				duration = (t2.tv_sec - t1.tv_sec) +
				    (t2.tv_usec - t1.tv_usec) / 1000000.0;
				NCFS_DATA->diskwrite_time += duration;
			}

			if (log_) {
				fprintf(cachelog_,
					"WB: %s, start %d, end %d, ret %d\n",
					CE->name, dirtystart, (i - 1), retstat);
			}
			dirtystart = -1;
			for (j = dirtystart; j < i; j++) {
				CE->dirtyblock[j] = CLEAN;
			}
		}
		//close(fd);
		CE->dirtybit = CLEAN;
	}
}

void *CacheLayer::Fetch_Thread_Func(void *data)
{
	Cache_Argument *CA = (Cache_Argument *) data;
	CacheLayer *This = CA->This;
	This->Fetch(CA->CE, CA->offset);
	return NULL;
}

/*            
 * DiskFlush: flush data to disk
 * 
 * @param offset: data offset of cache to be flushed.
 */
void CacheLayer::DiskFlush(long long offset)
{
	int i;
	//Cache_Argument CA[numofdisk_]; 

	if (log_) {
		fprintf(cachelog_, "+++Cache_DiskFlush: offset = %lld\n",
			offset);
	}
#ifdef MultiThread
	pthread_t tid[numofdisk_];
#endif
	for (i = 0; i < numofdisk_; i++) {
		//int diskfd;
		//if(msg_)printf("+++Cache_Fetch\n");          
		Cache_Entry *CE = diskcache_[i];
		//disk_id = CA->disk_id;
		//WriteBack(CE);                  
		// offset = CA->offset;
		//dev = CE->name;
		//diskfd = open(dev,O_RDONLY);   
		//printf("diskfd %d\n",diskfd);

#ifdef MultiThread
		Cache_Argument *CA;
		CA = (Cache_Argument *) malloc(sizeof(Cache_Argument));
		CA->CE = CE;
		CA->offset = offset;
		CA->This = this;
		pthread_create(&tid[i], NULL, Fetch_Thread_Func, CA);
#else
		Fetch(CE, offset);
#endif

		/*
		   memset(CE->content,0,CACHE_SIZE);                            
		   //retstat = pread(diskfd,CE->content,CACHE_SIZE,offset);
		   retstat = storageLayer->DiskRead(CE->name, (char*)CE->content,
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
		   } */
	}
#ifdef MultiThread
	for (i = 0; i < numofdisk_; ++i) {
		pthread_join(tid[i], NULL);
	}
#endif
}

/*
 * FileInfoFlush: flush file info to file       
 *       
 * @param id: id of the FileInfoCache (we use file descriptor for this)
 * @param offset: offset of cache starting to be flushed
 */
void CacheLayer::FileInfoFlush(int id, long long offset)
{
	if (msg_) {
		printf("+++Cache_FileInfoFlush: id %d, offset %lld\n", id,
		       offset);
	}
	if (log_) {
		fprintf(cachelog_, "FF: id = %d, offset = %lld\n", id, offset);
	}
	Fetch(fileinfocache_[id], offset);
}

/*****************************************************************************
 * Public functions
 *****************************************************************************/
/*
 * Constructor - create disk cache and file info cache
 *
 * @param ncfs_state: NCFS context state (with disk information)
 */
CacheLayer::CacheLayer(struct ncfs_state *ncfs_data)
{

	int block_size;
	int i;

	// XXX: pclee - should we allocate memory for a maximum number of
	// possible disks and dynamically associate new disk?

	// create the cache log         
	cachelog_ = fopen("cachelog", "w+");

	// create the disk cache based on the number of disks we have
	numofdisk_ = ncfs_data->disk_total_num;	// set the number of disks
	block_size = ncfs_data->chunk_size;
	diskcache_ =
	    (Cache_Entry **) malloc(sizeof(Cache_Entry *) * numofdisk_);
	for (i = 0; i < numofdisk_; i++) {
		diskcache_[i] = (Cache_Entry *) malloc(sizeof(Cache_Entry));
		diskcache_[i]->content = (void *)malloc(CACHE_SIZE);
		diskcache_[i]->dirtybit = CLEAN;
		diskcache_[i]->dirtyblock = (Cache_Status *)
		    calloc(CACHE_SIZE / block_size, sizeof(Cache_Status));
		diskcache_[i]->block_size = block_size;
		diskcache_[i]->startaddress = -1;
		diskcache_[i]->contentsize = 0;
		strncpy(diskcache_[i]->name, ncfs_data->dev_name[i], PATH_MAX);
		diskcache_[i]->id = i;
		if (msg_) {
			printf("+++Cache_DiskCreate disk = %d\n", i);
		}
		if (log_) {
			fprintf(cachelog_, "+++Cache_DiskCreate disk = %d\n",
				i);
		}
	}

	// create the file info cache (do not initiate name at this point)

	/* //Should create FileInfoCache on demand
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
	 */

	// init cache locks
	pthread_mutex_init(&diskcachelock_, NULL);
	for (i = 0; i < FILE_MAX; ++i) {
		fileinfocache_[i] = NULL;
		pthread_mutex_init(&filecachelock_[i], NULL);
	}
}

/*
 * DiskWrite: write data to disk
 * 
 * @param disk_id: disk id
 * @param buf: data buffer to be written to cache
 * @param size: data size of the data buffer
 * @param offset: offset of the data on the file to be written
 */
int CacheLayer::DiskWrite(int disk_id, const char *buf, long long size,
			  long long offset)
{

	struct timeval t1, t2;
	double duration;

	if (msg_) {
		printf
		    ("+++Cache_DiskWrite: disk = %d, buf = %ld, size = %lld, offset = %lld\n",
		     disk_id, (long int)buf, size, offset);
	}
	if (log_) {
		fprintf(cachelog_, "DW: disk %d, size %lld, offset %lld\n",
			disk_id, size, offset);
	}

	if (NCFS_DATA->no_cache == 0) {
		return GenericWrite(diskcache_[disk_id], buf, size, offset,
				    DISKCACHE);
	} else {
		//printf("**DiskWrite no_cache\n");
		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t1, NULL);
		}

		int fd = open(NCFS_DATA->dev_name[disk_id], O_WRONLY);
		int retstat = pwrite(fd, buf, size, offset);
		close(fd);

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t2, NULL);

			duration = (t2.tv_sec - t1.tv_sec) +
			    (t2.tv_usec - t1.tv_usec) / 1000000.0;
			NCFS_DATA->diskwrite_time += duration;
		}

		return retstat;
	}
}

/*
 * DiskRead: read data from disk
 * 
 * @param disk_id: disk id
 * @param buf: data buffer to be read from cache
 * @param size: data size of the data buffer
 * @param offset: offset of the data on the file to be read  
 */
int CacheLayer::DiskRead(int disk_id, char *buf, long long size,
			 long long offset)
{

	struct timeval t1, t2;
	double duration;

	if (msg_) {
		printf
		    ("+++Cache_DiskRead: disk = %d, buf = %ld, size = %lld, offset = %lld\n",
		     disk_id, (long int)buf, size, offset);
	}
	if (log_) {
		fprintf(cachelog_, "DR: disk %d, size %lld, offset %lld\n",
			disk_id, size, offset);
	}

	if (NCFS_DATA->no_cache == 0) {
		return GenericRead(diskcache_[disk_id], buf, size, offset,
				   DISKCACHE);
	} else {
		//printf("**DiskRead no_cache\n");
		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t1, NULL);
		}

		int fd = open(NCFS_DATA->dev_name[disk_id], O_RDONLY);
		int retstat = pread(fd, buf, size, offset);
		close(fd);

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t2, NULL);

			duration = (t2.tv_sec - t1.tv_sec) +
			    (t2.tv_usec - t1.tv_usec) / 1000000.0;
			NCFS_DATA->diskread_time += duration;
		}

		return retstat;
	}
}

/*
 * DiskDestroy: destroy disk cache
 */
void CacheLayer::DiskDestroy(void)
{
	int i;
	if (msg_) {
		printf("+++Cache_DiskDestroy\n");
	}
	if (log_) {
		fprintf(cachelog_, "+++Cache_DiskDestroy\n");
	}
	for (i = 0; i < numofdisk_; i++) {
		WriteBack(diskcache_[i]);
		//free(diskcache_[i]->content);
		//free(diskcache_[i]);
	}
	//free(diskcache_);
	return;
}

/*****
 * File Info Cache
 *****/

/*
 * Cache_FileInfoWrite: write file info to cache
 * 
 * @param id: id of the FileInfoCache (we use file descriptor for this)
 * @param buf: data buffer to be written to cache
 * @param size: size of the data buffer
 * @param offset: offset of the data on the file to be written
 * @return: size of successful write
 */
int CacheLayer::FileInfoWrite(int id, const char *buf, long long size,
			      long long offset)
{
	if (msg_) {
		printf
		    ("+++Cache_FileInfoWrite: id %d, size %lld, offset %lld\n",
		     id, size, offset);
	}
	if (log_) {
		fprintf(cachelog_, "FW: id %d, size %lld, offset %lld\n", id,
			size, offset);
	}
	if (id > FILE_MAX) {
		if (log_) {
			fprintf(cachelog_, "FW: id %d exceeed\n\r", id);
		}
		return -1;
	} else {
		return GenericWrite(fileinfocache_[id], buf, size, offset,
				    FILEINFOCACHE);
	}
}

/*
 * FileInfoRead: read file info from cache
 * 
 * @param id: id of the FileInfoCache (we use file descriptor for this)
 * @param buf: data buffer to be read from cache
 * @param size: size of the buffer
 * @param offset: offset of data on the file to be read
 * @return: size of successful read
 */
int CacheLayer::FileInfoRead(int id, char *buf, long long size,
			     long long offset)
{

	if (msg_) {
		printf("+++Cache_FileInfoRead: id %d, size %lld, offset %lld\n",
		       id, size, offset);
	}
	if (log_) {
		fprintf(cachelog_, "FR: id %d, size %lld, offset %lld\n", id,
			size, offset);
	}
	if (id > FILE_MAX) {
		if (log_) {
			fprintf(cachelog_, "FR: id %d exceeed\n\r", id);
		}
		return -1;
	} else {
		return GenericRead(fileinfocache_[id], (char *)buf, size,
				   offset, FILEINFOCACHE);
	}
}

/*
 * FileInfoCreate: create file info cache
 * 
 * @param path: path of file
 * @param id: id of the FileInfoCache (we use file descriptor for this)
 */
void CacheLayer::FileInfoCreate(const char *path, int id)
{
	if (msg_) {
		printf("+++Cache_FileInfoCreate: %s, id %d\n", path, id);
	}
	if (log_) {
		fprintf(cachelog_, "FC: path %s, id %d\n", path, id);
	}
	if (fileinfocache_[id] != NULL)
		FileInfoDestroy(id);

	fileinfocache_[id] = (Cache_Entry *) malloc(sizeof(Cache_Entry));
	fileinfocache_[id]->content = (void *)malloc(CACHE_SIZE);
	fileinfocache_[id]->dirtybit = CLEAN;
	fileinfocache_[id]->dirtyblock = (Cache_Status *)
	    calloc(CACHE_SIZE / sizeof(struct data_block_info),
		   sizeof(Cache_Status));
	fileinfocache_[id]->block_size = sizeof(struct data_block_info);
	fileinfocache_[id]->startaddress = -1;
	fileinfocache_[id]->contentsize = 0;
	strncpy(fileinfocache_[id]->name, path, PATH_MAX);
	fileinfocache_[id]->id = id;
	pthread_mutex_init(&filecachelock_[id], NULL);

	//fileinfocache_[id]->name (leave it alone until open)
	//fileinfocache_[i]->id = 0;
}

/*
 * FileInfoDestroy: destroy file info cache
 * 
 * @param id: id of the FileInfoCache (we use file descriptor for this)
 */
void CacheLayer::FileInfoDestroy(int id)
{
	if (fileinfocache_[id] != NULL) {
		if (msg_) {
			fprintf(stderr, "+++Cache_FileInfoDestroy: %s, id %d\n",
				fileinfocache_[id]->name, id);
		}
		if (log_) {
			fprintf(cachelog_,
				"+++Cache_FileInfoDestroy: %s, id %d\n",
				fileinfocache_[id]->name, id);
		}
		WriteBack(fileinfocache_[id]);
		free(fileinfocache_[id]->content);
		fileinfocache_[id] = NULL;
		pthread_mutex_destroy(&filecachelock_[id]);
	}
	return;
}

/*
 * FileInfoSearch: search file info
 * 
 * @param path: file path
 * @return: i >= 0 for success; -1 for error
 */
int CacheLayer::FileInfoSearch(const char *path)
{
	int i;
	for (i = 0; i < FILE_MAX; ++i) {
		if (fileinfocache_[i] != NULL) {
			if (strncmp(path, fileinfocache_[i]->name,
				    sizeof(fileinfocache_[i]->name)) == 0)
				return i;
		}
	}
	return -1;
}

void CacheLayer::DiskCacheName(int diskid, char *newdevice)
{
	memset(diskcache_[diskid]->name, 0, PATH_MAX);
	strncpy(diskcache_[diskid]->name, newdevice, strlen(newdevice));
	printf("Name updated %s|\n", diskcache_[diskid]->name);
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
