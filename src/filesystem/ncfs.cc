/*
Network coding filesystem (NCFS)

Version: 1.1.0
Date: October 2011

Authors: 
Lee Pak Ching (pclee@cse.cuhk.edu.hk)
Hu YuChong (yuchonghu@gmail.com)
Yu Chiu Man (cmyu@cse.cuhk.edu.hk)
Li Yan Kit (ykli7@cse.cuhk.edu.hk)
Kong Chun Ho (chkong8@cse.cuhk.edu.hk)
Zhu YunFeng (zyfl@mail.ustc.edu.cn)
Lui Chi Shing (cslui@cse.cuhk.edu.hk)
*/

/*
Mainly modified functions for NCFS:
ncfs_getattr()
ncfs_fgetattr()
ncfs_read()
ncfs_write()
ncfs_unlink()
ncfs_destroy()
*/

//#define _XOPEN_SOURCE 500
#include "filesystem_common.hh"
#include "filesystem_utils.hh"
#include "../cache/cache.hh"
#include "../coding/coding.hh"
#include "../storage/storage.hh"
#include "../gui/report.hh"
#include "../gui/diskusage_report.hh"
#include "../gui/process_report.hh"

//Add by zhuyunfeng on October 11, 2011 begin.
#include "../config/config.hh"
//Add by zhuyunfeng on October 11, 2011 end.

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

#include <fuse.h>
#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/xattr.h>

#include <sys/stat.h>

extern "C" {
#include "../jerasure/galois.h"
#include "../jerasure/jerasure.h"
#include "../jerasure/reed_sol.h"
}
//ncfs context state
struct ncfs_state *NCFS_DATA;
FileSystemLayer *fileSystemLayer;

//Add by zhuyunfeng on October 11, 2011 begin.
ConfigLayer* configLayer;
//Add by zhuyunfeng on October 11, 2011 end.

CacheLayer *cacheLayer;
StorageLayer *storageLayer;
ReportLayer *reportLayer;
DiskusageReport *diskusageLayer;
ProcessReport *processReport;
bool terminated = false;

//Add by zhuyunfeng on October 12, 2011 begin.
//for self-defined block size, actually the utility is developed by Michael.
char *bigtempbuf;
int bigtempbuf_maxcount;
int bigtempbuf_counter;
int bigblocksize;
int sysblocksize;
char *bigreadbuf;
char *bigreadpath;
long bigreadoffset;
int bigreadsize;
//Add by zhuyunfeng on October 12, 2011 end.

// Report errors and give -errno to caller
int ncfs_error(const char *str)
{
	int ret = -errno;

	//printf("    ERROR %s: %s\n", str, strerror(errno));

	return ret;
}

///////////////////////////////////////////////////////////
//
// Prototypes for all these functions, and the C-style comments,
// come indirectly from /usr/include/fuse.h
//
/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
/*
 * ncfs_getattr: Get file attribute by path
 *
 * @param path: File path
 * @param statbuf: output file attribute structure
 *
 *@return: 0 for success; -1 for error
 */
int ncfs_getattr(const char *path, struct stat *statbuf)
{
	int retstat = 0;
	char fpath[PATH_MAX];

	//start ncfs
	int retstat2 = 0;
	int value;
	int filefd;
	//end ncfs

	fileSystemLayer->ncfs_fullpath(fpath, path);

	retstat = lstat(fpath, statbuf);

	if (retstat != 0)
		retstat = ncfs_error("ncfs_getattr lstat");

	filefd = cacheLayer->FileInfoSearch(fpath);
	//Wilson: Thinking about a way to read from cache
	if (filefd == -1) {
		filefd = open(fpath, O_RDONLY);
		retstat2 = pread(filefd, (char *)&value, sizeof(int), 0);
		close(filefd);
	} else
		retstat2 =
		    cacheLayer->FileInfoRead(filefd, (char *)&value,
					     sizeof(int), 0);

	statbuf->st_size = value;

	return retstat;
}

/** Read the target of a symbolic link
 *
 * The buffer should be filled with a null terminated string.  The
 * buffer size argument includes the space for the terminating
 * null character.  If the linkname is too long to fit in the
 * buffer, it should be truncated.  The return value should be 0
 * for success.
 */
// the description given above doesn't correspond to the readlink(2)
// man page -- according to that, if the link is too long for the
// buffer, it ends up without the null termination
int ncfs_readlink(const char *path, char *link, size_t size)
{
	int retstat = 0;
	char fpath[PATH_MAX];

	fileSystemLayer->ncfs_fullpath(fpath, path);

	retstat = readlink(fpath, link, size);
	if (retstat < 0)
		retstat = ncfs_error("ncfs_readlink readlink");

	return 0;
}

/** Create a file node
 *
 * There is no create() operation, mknod() will be called for
 * creation of all non-directory, non-symlink nodes.
 */
// shouldn't that comment be "if" there is no.... ?
int ncfs_mknod(const char *path, mode_t mode, dev_t dev)
{
	int retstat = 0;
	char fpath[PATH_MAX];

	fileSystemLayer->ncfs_fullpath(fpath, path);

	// On Linux this could just be 'mknod(path, mode, rdev)' but this
	//  is more portable
	if (S_ISREG(mode)) {
		retstat = open(fpath, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (retstat < 0)
			retstat = ncfs_error("ncfs_mknod open");
		else {
			retstat = close(retstat);
			if (retstat < 0)
				retstat = ncfs_error("ncfs_mknod close");
		}
	} else if (S_ISFIFO(mode)) {
		retstat = mkfifo(fpath, mode);
		if (retstat < 0)
			retstat = ncfs_error("ncfs_mknod mkfifo");
	} else {
		retstat = mknod(fpath, mode, dev);
		if (retstat < 0)
			retstat = ncfs_error("ncfs_mknod mknod");
	}

	return retstat;
}

/** Create a directory */
int ncfs_mkdir(const char *path, mode_t mode)
{
	int retstat = 0;
	char fpath[PATH_MAX];

	fileSystemLayer->ncfs_fullpath(fpath, path);

	retstat = mkdir(fpath, mode);
	if (retstat < 0)
		retstat = ncfs_error("ncfs_mkdir mkdir");

	return retstat;
}

/** Remove a file */
/*
 * ncfs_unlink: remove a file; record deleted spaces
 *
 * @param path: file path
 *@return: 0 for success; -1 for error
 */
int ncfs_unlink(const char *path)
{
	int retstat = 0;
	char fpath[PATH_MAX];

	int i;
	int filefd;
	int offset;
	int retstat2;
	int file_size, file_block_num;
	struct data_block_info temp_info;

	fileSystemLayer->ncfs_fullpath(fpath, path);

	filefd = open(fpath, O_RDONLY);
	cacheLayer->FileInfoCreate(fpath, filefd);
	retstat2 =
	    cacheLayer->FileInfoRead(filefd, (char *)&file_size, sizeof(int),
				     0);
	//Wilson: Thinking about a way to read from cache
	file_block_num = file_size / NCFS_DATA->chunk_size;
	if ((file_size % NCFS_DATA->chunk_size) > 0) {
		file_block_num++;
	}

	for (i = 0; i < file_block_num; i++) {
		offset = sizeof(struct data_block_info) * (i + 1);
		//Wilson: Thinking about a way to read from cache
		retstat2 =
		    cacheLayer->FileInfoRead(filefd, (char *)&temp_info,
					     sizeof(struct data_block_info),
					     offset);
		retstat2 =
		    fileSystemLayer->space_list_add(temp_info.disk_id,
						    temp_info.block_no);
	}
	close(filefd);

	retstat = unlink(fpath);
	if (retstat < 0)
		retstat = ncfs_error("ncfs_unlink unlink");

	return retstat;
}

/** Remove a directory */
int ncfs_rmdir(const char *path)
{
	int retstat = 0;
	char fpath[PATH_MAX];

	fileSystemLayer->ncfs_fullpath(fpath, path);

	retstat = rmdir(fpath);
	if (retstat < 0)
		retstat = ncfs_error("ncfs_rmdir rmdir");

	return retstat;
}

/** Create a symbolic link */
// The parameters here are a little bit confusing, but do correspond
// to the symlink() system call.  The 'path' is where the link points,
// while the 'link' is the link itself.  So we need to leave the path
// unaltered, but insert the link into the mounted directory.
int ncfs_symlink(const char *path, const char *link)
{
	int retstat = 0;
	char flink[PATH_MAX];

	fileSystemLayer->ncfs_fullpath(flink, link);

	retstat = symlink(path, flink);
	if (retstat < 0)
		retstat = ncfs_error("ncfs_symlink symlink");

	return retstat;
}

/** Rename a file */
// both path and newpath are fs-relative
int ncfs_rename(const char *path, const char *newpath)
{
	int retstat = 0;
	char fpath[PATH_MAX];
	char fnewpath[PATH_MAX];

	fileSystemLayer->ncfs_fullpath(fpath, path);
	fileSystemLayer->ncfs_fullpath(fnewpath, newpath);

	retstat = rename(fpath, fnewpath);
	if (retstat < 0)
		retstat = ncfs_error("ncfs_rename rename");

	return retstat;
}

/** Create a hard link to a file */
int ncfs_link(const char *path, const char *newpath)
{
	int retstat = 0;
	char fpath[PATH_MAX], fnewpath[PATH_MAX];

	fileSystemLayer->ncfs_fullpath(fpath, path);
	fileSystemLayer->ncfs_fullpath(fnewpath, newpath);

	retstat = link(fpath, fnewpath);
	if (retstat < 0)
		retstat = ncfs_error("ncfs_link link");

	return retstat;
}

/** Change the permission bits of a file */
int ncfs_chmod(const char *path, mode_t mode)
{
	int retstat = 0;
	char fpath[PATH_MAX];

	fileSystemLayer->ncfs_fullpath(fpath, path);

	retstat = chmod(fpath, mode);
	if (retstat < 0)
		retstat = ncfs_error("ncfs_chmod chmod");

	return retstat;
}

/** Change the owner and group of a file */
int ncfs_chown(const char *path, uid_t uid, gid_t gid)
{
	int retstat = 0;
	char fpath[PATH_MAX];

	fileSystemLayer->ncfs_fullpath(fpath, path);

	retstat = chown(fpath, uid, gid);
	if (retstat < 0)
		retstat = ncfs_error("ncfs_chown chown");

	return retstat;
}

/** Change the size of a file */
int ncfs_truncate(const char *path, off_t newsize)
{
	int retstat = 0;
	char fpath[PATH_MAX];

	fileSystemLayer->ncfs_fullpath(fpath, path);

	retstat = truncate(fpath, newsize);
	if (retstat < 0)
		ncfs_error("ncfs_truncate truncate");

	return retstat;
}

/** Change the access and/or modification times of a file */
/* note -- I'll want to change this as soon as 2.6 is in debian testing */
int ncfs_utime(const char *path, struct utimbuf *ubuf)
{
	int retstat = 0;
	char fpath[PATH_MAX];

	fileSystemLayer->ncfs_fullpath(fpath, path);

	retstat = utime(fpath, ubuf);
	if (retstat < 0)
		retstat = ncfs_error("ncfs_utime utime");

	return retstat;
}

/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitrary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.
 *
 * Changed in version 2.2
 */
int ncfs_open(const char *path, struct fuse_file_info *fi)
{
	int retstat = 0;
	int fd;
	char fpath[PATH_MAX];

	fileSystemLayer->ncfs_fullpath(fpath, path);

	fd = open(fpath, fi->flags);
	if (fd < 0)
		retstat = ncfs_error("ncfs_open open");

	fi->fh = fd;
	cacheLayer->FileInfoCreate(fpath, fd);

	return 0;
}

/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes.  An exception to this is when the
 * 'direct_io' mount option is specified, in which case the return
 * value of the read system call will reflect the return value of
 * this operation.
 *
 * Changed in version 2.2
 */
// I don't fully understand the documentation above -- it doesn't
// match the documentation for the read() system call which says it
// can return with anything up to the amount of data requested. nor
// with the fusexmp code which returns the amount of data also
// returned by read.
/*
 * ncfs_read: Handle read request
 *
 * @param path: File path
 * @param buf: data buffer
 * @param size: size of requested data
 * @param offset: file offset
 * @fi: file information provided by fuse
 *
 * @return: size of data read
 */
int ncfs_read(const char *path, char *buf, size_t size, off_t offset,
	      struct fuse_file_info *fi)
{
	int retstat = 0;

	int addr_offset, another_addr_offset;
	int block_size;
	size_t bytes_read_already, blocks_read_already, read_size;
	char temp_buf[NCFS_DATA->chunk_size];
	char temp_another_buf[NCFS_DATA->chunk_size];
	int i, j;
	struct data_block_info temp_block_info;
	struct data_block_info temp_another_block_info;
	struct data_block_info temp_block_info_list[64];
	char *temp_ptr;
	int sequential_read;

	if (NCFS_DATA->operation_mode == 2) {
		printf("***ncfs_read: incapable mode: cannot read file.\n");
		return -1;
	}

	block_size = NCFS_DATA->chunk_size;
	
	if (size <= (size_t) block_size) {
	  
		off_t dest_offset=offset+size;
		
		if((dest_offset / block_size)==(offset / block_size)){
		  
			addr_offset =
			    ((int)(offset / block_size) +
			    1) * sizeof(struct data_block_info);
			    
			retstat =
			    cacheLayer->FileInfoRead(fi->fh, (char *)&temp_block_info,
						    sizeof(struct data_block_info),
						    addr_offset);

			//--Wilson Edit Start
			if (retstat == 0)
				return 0;
			//--Wilson Edit End
			
			retstat =
			  fileSystemLayer->codingLayer->decode(temp_block_info.
								disk_id, temp_buf, block_size,
								(temp_block_info.
								  block_no) *
								block_size);
			  
			memcpy(buf, (char *)&temp_buf[(int)(offset%block_size)], size);
			
			//--Wilson Edit End
			//bytes_read_already = retstat;
			bytes_read_already=size;		  
		  
		  
		}else{
			addr_offset =
			    ((int)(offset / block_size) +
			    1) * sizeof(struct data_block_info);
			    
			another_addr_offset = 
			    ((int)(dest_offset / block_size) +
			    1) * sizeof(struct data_block_info);
			    
			size_t temp_foffset= (size_t)block_size- (offset % (size_t)block_size);
			
			retstat =
			    cacheLayer->FileInfoRead(fi->fh, (char *)&temp_block_info,
						    sizeof(struct data_block_info),
						    addr_offset);
			retstat =
			    cacheLayer->FileInfoRead(fi->fh, (char *)&temp_another_block_info,
						    sizeof(struct data_block_info),
						    another_addr_offset);
			    
			//--Wilson Edit Start
			if (retstat == 0)
				return 0;
			//--Wilson Edit End

			//noraml mode, or degraded mode but reading healthy disk
			//--Wilson Edit Start
						
	// 		retstat =
	// 		   fileSystemLayer->codingLayer->decode(temp_block_info.
	// 							 disk_id, buf, size,
	// 							 (temp_block_info.
	// 							  block_no) *
	// 							 block_size);
			
			retstat =
			  fileSystemLayer->codingLayer->decode(temp_block_info.
								disk_id, temp_buf, block_size,
								(temp_block_info.
								  block_no) *
								block_size);
			retstat =
			  fileSystemLayer->codingLayer->decode(temp_another_block_info.
								disk_id, temp_another_buf, block_size,
								(temp_another_block_info.
								  block_no) *
								block_size);
			  
			memcpy(buf, (char *)&temp_buf[(int)(offset%block_size)], temp_foffset);
			
			temp_ptr = (char *)&buf[temp_foffset];
				
			memcpy(temp_ptr, temp_another_buf, size-temp_foffset);
							
			//--Wilson Edit End
			//bytes_read_already = retstat;
			bytes_read_already=size;			  
		  
		}

	} else {		//if size > block_size
		bytes_read_already = 0;
		blocks_read_already = 0;
		offset = (int)(offset / block_size) + 1;

		i = 0;
		while (bytes_read_already < size) {
			addr_offset =
			    (offset +
			     blocks_read_already) *
			    sizeof(struct data_block_info);
			retstat =
			    cacheLayer->FileInfoRead(fi->fh,
						     (char *)
						     &temp_block_info_list[i],
						     sizeof(struct
							    data_block_info),
						     addr_offset);

			//calculte size of read for this block
			read_size = size - bytes_read_already;
			if (read_size > (size_t) block_size) {
				read_size = block_size;
			}

			bytes_read_already = bytes_read_already + read_size;
			blocks_read_already++;
			i++;
		}

		sequential_read = 1;	//default true
		j = 1;
		while ((j < i) && (sequential_read == 0)) {
			if ((temp_block_info_list[j].disk_id !=
			     temp_block_info_list[j - 1].disk_id)
			    || (temp_block_info_list[j - 1].block_no !=
				temp_block_info_list[j].block_no - 1)) {
				sequential_read = 1;
			}
			j++;
		}

		if (sequential_read == 0) {
			//sequential read
			//--Wilson Edit Start
			retstat =
			    fileSystemLayer->codingLayer->
			    decode(temp_block_info_list[0].disk_id, buf, size,
				   (temp_block_info_list[0].block_no) *
				   block_size);
			bytes_read_already = retstat;
			//--Wilson Edit End
		} else {
			//non-sequential read

			bytes_read_already = 0;
			blocks_read_already = 0;

			i = 0;
			while (bytes_read_already < size) {

				//calculte size of read for this block
				read_size = size - bytes_read_already;
				if (read_size > (size_t) block_size) {
					read_size = block_size;
				}
				retstat =
				    fileSystemLayer->codingLayer->
				    decode(temp_block_info_list[i].disk_id,
					   temp_buf, read_size,
					   (temp_block_info_list[i].block_no) *
					   block_size);

				temp_ptr = (char *)&buf[bytes_read_already];
				memcpy(temp_ptr, temp_buf, read_size);

				bytes_read_already =
				    bytes_read_already + read_size;
				blocks_read_already++;
				i++;
			}
		}
	}

	//--Wilson Edit Start
	if (bytes_read_already < 0)
		bytes_read_already = ncfs_error("ncfs_read read");
	return bytes_read_already;
	//--Wilson Edit End
}

/** Write data to an open file
 *
 * Write should return exactly the number of bytes requested
 * except on error.  An exception to this is when the 'direct_io'
 * mount option is specified (see read operation).
 *
 * Changed in version 2.2
 */
/*
 * ncfs_write: Handle write request
 *
 * @param path: File path
 * @param buf: data buffer
 * @param size: size of requested data
 * @param offset: file offset
 * @fi: file information provided by fuse
 *
 * @return: size of data written
 */
int ncfs_write(const char *path, const char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi)
{
	int retstat = 0;

	//int data_block_no;
	int addr_offset;
	int file_size;
	int block_size;
	struct data_block_info temp_block_info;

	size_t size_to_write;
	size_t temp_size;
	off_t temp_offset;
	char *temp_buf;

	//Wilson: We can write file even in degraded
	if (NCFS_DATA->operation_mode == 1){
		printf("***ncfs_read: degraded mode: cannot write file.\n");
		return -1;
	}
	else if (NCFS_DATA->operation_mode == 2){
		printf("***ncfs_read: incapable mode: cannot write file.\n");
		return -1;
	}

	block_size = NCFS_DATA->chunk_size;
	
	//operate with big block size
	if (NCFS_DATA->usebigblock == 1){
	  
		//if (size <= (size_t)block_size){
		if (size <= (size_t)sysblocksize){
			
			//for self-defined block size
			memcpy(bigtempbuf+(bigtempbuf_maxcount-bigtempbuf_counter)*sysblocksize, buf, size);
			bigtempbuf_counter--;
			if (bigtempbuf_counter <= 0){
				//printf("******Bigtempbuf full: encode data.\n");
				temp_size = (bigtempbuf_maxcount - 1)*sysblocksize + size;
				temp_block_info = fileSystemLayer->codingLayer->encode(bigtempbuf, temp_size); 
				//cacheLayer->writeFileCache(fi->fh,buf,temp_size,offset);

				file_size = offset + temp_size - (bigblocksize - sysblocksize);
				addr_offset = ((int)(offset / bigblocksize) + 1) * sizeof(struct data_block_info);

				retstat = cacheLayer->FileInfoWrite(fi->fh, (char*)&file_size, sizeof(int), 0);
				retstat = cacheLayer->FileInfoWrite(fi->fh, (char*)&temp_block_info,
								sizeof(struct data_block_info), addr_offset);

				memset(bigtempbuf,0,bigblocksize);
				bigtempbuf_counter = bigtempbuf_maxcount;
			}
			else {
				//Add by zhuyunfeng on Oct 20, 2011 begin.
				
				//Write will be at least sysblocksize, if not, then we think it the end of the file
				if((size % (size_t)sysblocksize) > 0){
				  
				    //printf("******Bigtempbuf full: encode data.\n");
				    temp_size = (bigtempbuf_maxcount - 1)*sysblocksize + size;
				    temp_block_info = fileSystemLayer->codingLayer->encode(bigtempbuf, temp_size); 
				    //cacheLayer->writeFileCache(fi->fh,buf,temp_size,offset);

				    file_size = offset + temp_size - (bigblocksize - sysblocksize);
				    addr_offset = ((int)(offset / bigblocksize) + 1) * sizeof(struct data_block_info);

				    retstat = cacheLayer->FileInfoWrite(fi->fh, (char*)&file_size, sizeof(int), 0);
				    retstat = cacheLayer->FileInfoWrite(fi->fh, (char*)&temp_block_info,
								    sizeof(struct data_block_info), addr_offset);

				    memset(bigtempbuf,0,bigblocksize);
				    bigtempbuf_counter = bigtempbuf_maxcount;				  
				  
				}
				//Add by zhuyunfeng on Oct 20, 2011 end.
			}

		}
		else{
			//Support big_writes
			//temp_buf = (char*)malloc(sysblocksize * sizeof(char));
			size_to_write = size;
			temp_offset = offset;

			while (size_to_write > 0){
				if (size_to_write <= (size_t)sysblocksize){
					temp_size = size_to_write;
					size_to_write = 0;
				}
				else{
					temp_size = (size_t)sysblocksize;
					size_to_write = size_to_write - (size_t)sysblocksize;
				}

				//memcpy(temp_buf, buf+(size-size_to_write-temp_size)*sizeof(char), temp_size);
				//memcpy(bigtempbuf+(bigtempbuf_maxcount-bigtempbuf_counter)*sysblocksize, buf, size);

				memcpy(bigtempbuf+(bigtempbuf_maxcount-bigtempbuf_counter)*sysblocksize, 
					buf+(size-size_to_write-temp_size)*sizeof(char), temp_size);

				bigtempbuf_counter--;
				if (bigtempbuf_counter <= 0){
					printf("******Bigwrite Bigtempbuf full: encode data.\n");

					temp_size = (bigtempbuf_maxcount - 1)*sysblocksize + temp_size;
					temp_block_info = fileSystemLayer->codingLayer->encode(bigtempbuf, temp_size); 
					//cacheLayer->writeFileCache(fi->fh,buf,temp_size,offset);

					file_size = temp_offset + temp_size;
					addr_offset = ((int)(temp_offset / bigblocksize) + 1) 
							* sizeof(struct data_block_info);

					retstat = cacheLayer->FileInfoWrite(fi->fh, (char*)&file_size, sizeof(int), 0);
					retstat = cacheLayer->FileInfoWrite(fi->fh, (char*)&temp_block_info,
									sizeof(struct data_block_info), addr_offset);

					temp_offset = temp_offset + (off_t)temp_size;
					memset(bigtempbuf,0,bigblocksize);
					bigtempbuf_counter = bigtempbuf_maxcount;
				}
				else {
					printf("******Else bigwrite bigtempbuf\n");
				}			
			}

		}
	}
	else{
		if (size <= (size_t)block_size){
			//Original program segment of single-block write:
			temp_block_info = fileSystemLayer->codingLayer->encode(buf, size);

			file_size = offset + size;
			addr_offset = ((int)(offset / block_size) + 1) * sizeof(struct data_block_info);

			retstat = cacheLayer->FileInfoWrite(fi->fh, (char*)&file_size, sizeof(int), 0);
			//retstat = pwrite(fi->fh, (char *)&file_size, sizeof(int), 0);
			retstat = cacheLayer->FileInfoWrite(fi->fh, (char*)&temp_block_info,
							sizeof(struct data_block_info), addr_offset);
			//retstat = pwrite(fi->fh, (char *)&temp_block_info, sizeof(struct data_block_info), addr_offset);
			//printf("***ncfs_write 1: disk_id=%d, block_no = %d, retstat = %d\n",
			//		temp_block_info.disk_id, temp_block_info.block_no, retstat);
		}
		else{
			//Support big_writes
			temp_buf = (char*)malloc(sysblocksize * sizeof(char));
			size_to_write = size;
			temp_offset = offset;

			//Original program segment of multiple-block write:
			while (size_to_write > 0){
				if (size_to_write <= (size_t)block_size){
					temp_size = size_to_write;
					size_to_write = 0;
				}
				else{
					temp_size = (size_t)block_size;
					size_to_write = size_to_write - (size_t)block_size;
				}

				memcpy(temp_buf, buf+(size-size_to_write-temp_size)*sizeof(char), temp_size);

		        	temp_block_info = fileSystemLayer->codingLayer->encode(temp_buf, temp_size); 
		        	//cacheLayer->writeFileCache(fi->fh,temp_buf,temp_size,temp_offset);

		        	file_size = temp_offset + temp_size;
		        	addr_offset = ((int)(temp_offset / block_size) + 1) * sizeof(struct data_block_info);

		        	retstat = cacheLayer->FileInfoWrite(fi->fh, (char*)&file_size, sizeof(int), 0);
		        	retstat = cacheLayer->FileInfoWrite(fi->fh, (char*)&temp_block_info,
							sizeof(struct data_block_info), addr_offset);
				temp_offset = temp_offset + (off_t)temp_size;
			}

			free(temp_buf);
		}
	}

	retstat = size;

	if (retstat < 0)
		retstat = ncfs_error("ncfs_write pwrite");

	return retstat;
}


/** Get file system statistics
 *
 * The 'f_frsize', 'f_favail', 'f_fsid' and 'f_flag' fields are ignored
 *
 * Replaced 'struct statfs' parameter with 'struct statvfs' in
 * version 2.5
 */
int ncfs_statfs(const char *path, struct statvfs *statv)
{
	int retstat = 0;
	char fpath[PATH_MAX];

	fileSystemLayer->ncfs_fullpath(fpath, path);

	// get stats for underlying filesystem
	retstat = statvfs(fpath, statv);
	if (retstat < 0)
		retstat = ncfs_error("ncfs_statfs statvfs");

	return retstat;
}

/** Possibly flush cached data
 *
 * BIG NOTE: This is not equivalent to fsync().  It's not a
 * request to sync dirty data.
 *
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.  Since many applications ignore close()
 * errors this is not always useful.
 *
 * NOTE: The flush() method may be called more than once for each
 * open().  This happens if more than one file descriptor refers
 * to an opened file due to dup(), dup2() or fork() calls.  It is
 * not possible to determine if a flush is final, so each flush
 * should be treated equally.  Multiple write-flush sequences are
 * relatively rare, so this shouldn't be a problem.
 *
 * Filesystems shouldn't assume that flush will always be called
 * after some writes, or that if will be called at all.
 *
 * Changed in version 2.2
 */
int ncfs_flush(const char *path, struct fuse_file_info *fi)
{
	int retstat = 0;

	return retstat;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 * Changed in version 2.2
 */
int ncfs_release(const char *path, struct fuse_file_info *fi)
{
	int retstat = 0;

	cacheLayer->FileInfoDestroy(fi->fh);
	//close(fi->fh);

	//FILE *time_file;

// 	printf("Encoding time: %lf\n", NCFS_DATA->encoding_time);
// 	printf("Decoding time: %lf\n", NCFS_DATA->decoding_time);
// 	printf("Disk Read time: %lf\n", NCFS_DATA->diskread_time);
// 	printf("Disk Write time: %lf\n", NCFS_DATA->diskwrite_time);

	//print time measurement counters
	/*
	if ((time_file = fopen("time_data", "w+")) != NULL) {
		fprintf(time_file, "Encoding time: %lf\n",
			NCFS_DATA->encoding_time);
		fprintf(time_file, "Decoding time: %lf\n",
			NCFS_DATA->decoding_time);
		fprintf(time_file, "Disk Read time: %lf\n",
			NCFS_DATA->diskread_time);
		fprintf(time_file, "Disk Write time: %lf\n",
			NCFS_DATA->diskwrite_time);
		fclose(time_file);
	}
	*/

	return retstat;
}

/** Synchronize file contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data.
 *
 * Changed in version 2.2
 */
int ncfs_fsync(const char *path, int datasync, struct fuse_file_info *fi)
{
	int retstat = 0;

	if (datasync)
		retstat = fdatasync(fi->fh);
	else
		retstat = fsync(fi->fh);

	if (retstat < 0)
		ncfs_error("ncfs_fsync fsync");

	return retstat;
}

/** Set extended attributes */
int ncfs_setxattr(const char *path, const char *name, const char *value,
		  size_t size, int flags)
{
	int retstat = 0;
	char fpath[PATH_MAX];

	fileSystemLayer->ncfs_fullpath(fpath, path);

	retstat = lsetxattr(fpath, name, value, size, flags);
	if (retstat < 0)
		retstat = ncfs_error("ncfs_setxattr lsetxattr");

	return retstat;
}

/** Get extended attributes */
int ncfs_getxattr(const char *path, const char *name, char *value, size_t size)
{
	int retstat = 0;
	char fpath[PATH_MAX];

	fileSystemLayer->ncfs_fullpath(fpath, path);

	retstat = lgetxattr(fpath, name, value, size);
	if (retstat < 0)
		retstat = ncfs_error("ncfs_getxattr lgetxattr");

	return retstat;
}

/** List extended attributes */
int ncfs_listxattr(const char *path, char *list, size_t size)
{
	int retstat = 0;
	char fpath[PATH_MAX];
	//char *ptr;

	fileSystemLayer->ncfs_fullpath(fpath, path);

	retstat = llistxattr(fpath, list, size);
	if (retstat < 0)
		retstat = ncfs_error("ncfs_listxattr llistxattr");

	return retstat;
}

/** Remove extended attributes */
int ncfs_removexattr(const char *path, const char *name)
{
	int retstat = 0;
	char fpath[PATH_MAX];

	fileSystemLayer->ncfs_fullpath(fpath, path);

	retstat = lremovexattr(fpath, name);
	if (retstat < 0)
		retstat = ncfs_error("ncfs_removexattr lrmovexattr");

	return retstat;
}

/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this  directory
 *
 * Introduced in version 2.3
 */
int ncfs_opendir(const char *path, struct fuse_file_info *fi)
{
	DIR *dp;
	int retstat = 0;
	char fpath[PATH_MAX];

	fileSystemLayer->ncfs_fullpath(fpath, path);

	dp = opendir(fpath);
	if (dp == NULL)
		retstat = ncfs_error("ncfs_opendir opendir");

	fi->fh = (intptr_t) dp;

	return retstat;
}

/** Read directory
 *
 * This supersedes the old getdir() interface.  New applications
 * should use this.
 *
 * The filesystem may choose between two modes of operation:
 *
 * 1) The readdir implementation ignores the offset parameter, and
 * passes zero to the filler function's offset.  The filler
 * function will not return '1' (unless an error happens), so the
 * whole directory is read in a single readdir operation.  This
 * works just like the old getdir() method.
 *
 * 2) The readdir implementation keeps track of the offsets of the
 * directory entries.  It uses the offset parameter and always
 * passes non-zero offset to the filler function.  When the buffer
 * is full (or an error happens) the filler function will return
 * '1'.
 *
 * Introduced in version 2.3
 */
int ncfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		 off_t offset, struct fuse_file_info *fi)
{
	int retstat = 0;
	DIR *dp;
	struct dirent *de;

	// once again, no need for fullpath -- but note that I need to cast fi->fh
	dp = (DIR *) (uintptr_t) fi->fh;

	// Every directory contains at least two entries: . and ..  If my
	// first call to the system readdir() returns NULL I've got an
	// error; near as I can tell, that's the only condition under
	// which I can get an error from readdir()
	de = readdir(dp);
	if (de == 0)
		return -errno;

	// This will copy the entire directory into the buffer.  The loop exits
	// when either the system readdir() returns NULL, or filler()
	// returns something non-zero.  The first case just means I've
	// read the whole directory; the second means the buffer is full.
	do {
		if (filler(buf, de->d_name, NULL, 0) != 0)
			return -ENOMEM;
	} while ((de = readdir(dp)) != NULL);

	return retstat;
}

/** Release directory
 *
 * Introduced in version 2.3
 */
int ncfs_releasedir(const char *path, struct fuse_file_info *fi)
{
	int retstat = 0;

	closedir((DIR *) (uintptr_t) fi->fh);

	return retstat;
}

/** Synchronize directory contents
 *
 * If the datasync parameter is non-zero, then only the user data
 * should be flushed, not the meta data
 *
 * Introduced in version 2.3
 */
// when exactly is this called?  when a user calls fsync and it
// happens to be a directory? ???
int ncfs_fsyncdir(const char *path, int datasync, struct fuse_file_info *fi)
{
	int retstat = 0;

	return retstat;
}

/**
 * Initialize filesystem
 *
 * The return value will passed in the private_data field of
 * fuse_context to all file operations and as a parameter to the
 * destroy() method.
 *
 * Introduced in version 2.3
 * Changed in version 2.6
 */
// Undocumented but extraordinarily useful fact:  the fuse_context is
// set up before this function is called, and
// fuse_get_context()->private_data returns the user_data passed to
// fuse_main().  Really seems like either it should be a third
// parameter coming in here, or else the fact should be documented
// (and this might as well return void, as it did in older versions of
// FUSE).
void *ncfs_init(struct fuse_conn_info *conn)
{

	//for self-defined block size
	bigblocksize = NCFS_DATA->chunk_size;
	sysblocksize = 4096;

	if (bigblocksize > sysblocksize){
		NCFS_DATA->usebigblock = 1;
	}
	else{
		NCFS_DATA->usebigblock = 0;
	}

	if (NCFS_DATA->usebigblock == 1){
		bigtempbuf = (char *)calloc(sizeof(char)*bigblocksize,1);
		bigtempbuf_maxcount = bigblocksize / sysblocksize;
		bigtempbuf_counter = bigtempbuf_maxcount;

		bigreadbuf = (char *)calloc(128*sizeof(char)*bigblocksize,1);
		bigreadoffset = 0;
		bigreadsize = 0;
	}
	
	return NCFS_DATA;
}

/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 * Introduced in version 2.3
 */
/*
 * ncfs_destroy: Handle umount request; write metadata to system file
 *
 * @param userdata: userdata
 */
void ncfs_destroy(void *userdata)
{
	int i;
	int fd;
	int magic_no;
	int disk_total_num;
	int block_size;
	int space_list_num;
	int offset;
	int offset_space;
	int retstat;
	struct raid_metadata metadata;
	struct space_info temp_space_info;
	struct space_list *space_node;
	//end ncfs

// 	FILE *time_file;
// 
// 	printf("Encoding time: %lf\n", NCFS_DATA->encoding_time);
// 	printf("Decoding time: %lf\n", NCFS_DATA->decoding_time);
// 	printf("Disk Read time: %lf\n", NCFS_DATA->diskread_time);
// 	printf("Disk Write time: %lf\n", NCFS_DATA->diskwrite_time);
// 
// 	//print time measurement counters
// 	if ((time_file = fopen("time_data", "w+")) != NULL) {
// 		fprintf(time_file, "Encoding time: %lf\n",
// 			NCFS_DATA->encoding_time);
// 		fprintf(time_file, "Decoding time: %lf\n",
// 			NCFS_DATA->decoding_time);
// 		fprintf(time_file, "Disk Read time: %lf\n",
// 			NCFS_DATA->diskread_time);
// 		fprintf(time_file, "Disk Write time: %lf\n",
// 			NCFS_DATA->diskwrite_time);
// 		fclose(time_file);
// 	}

	//start ncfs
	//write metadata (disk free offset and size) to disk    
	fd = open("raid_metadata", O_WRONLY);
	magic_no = MAGIC_NUMBER;
	retstat = pwrite(fd, (char *)&magic_no, sizeof(int), 0);

	disk_total_num = NCFS_DATA->disk_total_num;
	block_size = NCFS_DATA->chunk_size;
	for (i = 0; i < disk_total_num; i++) {
		metadata.disk_id = i;
		metadata.free_offset = NCFS_DATA->free_offset[i] * block_size;
		metadata.free_size = (NCFS_DATA->free_size[i]) * block_size;
		offset = sizeof(struct raid_metadata) * (i + 1);
		retstat =
		    pwrite(fd, (char *)&metadata, sizeof(struct raid_metadata),
			   offset);
	}

	//write space info (deleted spaces) to disk

	offset_space = sizeof(struct raid_metadata) * (disk_total_num + 1);
	space_list_num = NCFS_DATA->space_list_num;

	retstat =
	    pwrite(fd, (char *)&space_list_num, sizeof(int), offset_space);

	space_node = NCFS_DATA->space_list_head;
	i = 0;
	while ((i < space_list_num) && (space_node != NULL)) {
		temp_space_info.disk_id = space_node->disk_id;
		temp_space_info.disk_block_no = space_node->disk_block_no;
		offset = offset_space + sizeof(struct space_info) * (i + 1);
		retstat =
		    pwrite(fd, (char *)&temp_space_info,
			   sizeof(struct space_info), offset);
		//printf("***ncfs_destroy: i=%d, retstat=%d\n",i,retstat);
		i++;
		space_node = space_node->next;
	}

	close(fd);

	fileSystemLayer->update_settting();

	if (NCFS_DATA->no_gui == 0) {
		reportLayer->SendAll();
	}

	cacheLayer->DiskDestroy();
	terminated = true;
}

/**
 * Check file access permissions
 *
 * This will be called for the access() system call.  If the
 * 'default_permissions' mount option is given, this method is not
 * called.
 *
 * This method is not called under Linux kernel versions 2.4.x
 *
 * Introduced in version 2.5
 */
int ncfs_access(const char *path, int mask)
{
	int retstat = 0;
	char fpath[PATH_MAX];

	fileSystemLayer->ncfs_fullpath(fpath, path);

	retstat = access(fpath, mask);

	if (retstat < 0)
		retstat = ncfs_error("ncfs_access access");

	return retstat;
}

/**
 * Create and open a file
 *
 * If the file does not exist, first create it with the specified
 * mode, and then open it.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the mknod() and open() methods
 * will be called instead.
 *
 * Introduced in version 2.5
 */
int ncfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	int retstat = 0;
	char fpath[PATH_MAX];
	int fd;

	fileSystemLayer->ncfs_fullpath(fpath, path);

	fd = creat(fpath, mode);
	if (fd < 0)
		retstat = ncfs_error("ncfs_create creat");

	fi->fh = fd;
	cacheLayer->FileInfoCreate(fpath, fd);

	return retstat;
}

/**
 * Change the size of an open file
 *
 * This method is called instead of the truncate() method if the
 * truncation was invoked from an ftruncate() system call.
 *
 * If this method is not implemented or under Linux kernel
 * versions earlier than 2.6.15, the truncate() method will be
 * called instead.
 *
 * Introduced in version 2.5
 */
int ncfs_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi)
{
	int retstat = 0;

	retstat = ftruncate(fi->fh, offset);
	if (retstat < 0)
		retstat = ncfs_error("ncfs_ftruncate ftruncate");

	return retstat;
}

/**
 * Get attributes from an open file
 *
 * This method is called instead of the getattr() method if the
 * file information is available.
 *
 * Currently this is only called after the create() method if that
 * is implemented (see above).  Later it may be called for
 * invocations of fstat() too.
 *
 * Introduced in version 2.5
 */
// Since it's currently only called after bb_create(), and bb_create()
// opens the file, I ought to be able to just use the fd and ignore
// the path...
int ncfs_fgetattr(const char *path, struct stat *statbuf,
		  struct fuse_file_info *fi)
{
	int retstat = 0;

	int retstat2 = 0;
	int value;

	retstat = fstat(fi->fh, statbuf);

	char fpath[PATH_MAX];
	fileSystemLayer->ncfs_fullpath(fpath, path);
	if (cacheLayer->FileInfoSearch(fpath) == -1)
		cacheLayer->FileInfoCreate(fpath, fi->fh);
	retstat2 =
	    cacheLayer->FileInfoRead(fi->fh, (char *)&value, sizeof(int), 0);
	statbuf->st_size = value;

	if (retstat < 0)
		retstat = ncfs_error("ncfs_fgetattr fstat");

	return retstat;
}

struct ncfs_fuse_operations:fuse_operations {
	ncfs_fuse_operations() {
		getattr = ncfs_getattr;	//ncfs modified
		readlink = ncfs_readlink;
		// no .getdir -- that's deprecated
		getdir = NULL;
		mknod = ncfs_mknod;
		mkdir = ncfs_mkdir;
		unlink = ncfs_unlink;	//ncfs modified
		rmdir = ncfs_rmdir;
		symlink = ncfs_symlink;
		rename = ncfs_rename;
		link = ncfs_link;
		chmod = ncfs_chmod;
		chown = ncfs_chown;
		truncate = ncfs_truncate;
		utime = ncfs_utime;
		open = ncfs_open;
		read = ncfs_read;	//ncfs modified
		write = ncfs_write;	//ncfs modified
		/** Just a placeholder, don't set */// huh???
		statfs = ncfs_statfs;
		flush = ncfs_flush;
		release = ncfs_release;
		fsync = ncfs_fsync;
		setxattr = ncfs_setxattr;
		getxattr = ncfs_getxattr;
		listxattr = ncfs_listxattr;
		removexattr = ncfs_removexattr;
		opendir = ncfs_opendir;
		readdir = ncfs_readdir;
		releasedir = ncfs_releasedir;
		fsyncdir = ncfs_fsyncdir;
		init = ncfs_init;
		destroy = ncfs_destroy;	//ncfs modifed
		access = ncfs_access;
		create = ncfs_create;
		ftruncate = ncfs_ftruncate;
		fgetattr = ncfs_fgetattr;	//ncfs modified
}};

static struct ncfs_fuse_operations ncfs_oper;

void ncfs_usage()
{
	fprintf(stderr, "usage:  ncfs rootDir mountPoint\n");
	abort();
}

int main(int argc, char *argv[])
{
	int i;
	int fuse_stat;
	struct ncfs_state *ncfs_data;
	int n, k, d;
	int field_power;
	int j;

	ncfs_data = (struct ncfs_state *)calloc(sizeof(struct ncfs_state), 1);
	if (ncfs_data == NULL) {
		perror("main calloc");
		abort();
	}

	fileSystemLayer = new FileSystemLayer();
	
	//Add by zhuyunfeng on October 11, 2011 begin.
	configLayer = new ConfigLayer();
	//Add by zhuyunfeng on October 11, 2011 end.

	// libfuse is able to do most of the command line parsing; all I
	// need to do is to extract the rootdir; this will be the first
	// non-option passed in.  I'm using the GNU non-standard extension
	// and having realpath malloc the space for the path
	// the string.
	for (i = 1; (i < argc) && (argv[i][0] == '-'); i++) ;
	if (i == argc)
		ncfs_usage();

	ncfs_data->rootdir = realpath(argv[i], NULL);
	ncfs_data->mountdir = realpath(argv[i + 1], NULL);

	ncfs_data->no_cache = 0;
	ncfs_data->no_gui = 0;
	ncfs_data->run_experiment = 0;

	ncfs_data->process_state = 0;
	ncfs_data->encoding_time = 0;
	ncfs_data->decoding_time = 0;
	ncfs_data->diskread_time = 0;
	ncfs_data->diskwrite_time = 0;

	ncfs_data->space_list_num = 0;	//for deletion
	ncfs_data->space_list_head = NULL;

	//Modified by zhuyunfeng on October 11, 2011 begin.
	fileSystemLayer->readSystemConfig(ncfs_data);
	
	//fileSystemLayer->get_raid_setting(ncfs_data);

	//for (j = 0; j < ncfs_data->disk_total_num; j++) {
	//	ncfs_data->disk_size[j] = (ncfs_data->free_size[j]);
	//}
	
        //Modified by zhuyunfeng on October 11, 2011 end.

	fileSystemLayer->get_raid_metadata(ncfs_data);

	for (j = 0; j < ncfs_data->disk_total_num; j++) {
		ncfs_data->free_offset[j] =
		    (ncfs_data->free_offset[j]) / (ncfs_data->chunk_size);
		ncfs_data->free_size[j] =
		    (ncfs_data->free_size[j]) / (ncfs_data->chunk_size);
		ncfs_data->disk_size[j] =
		    (ncfs_data->disk_size[j]) / (ncfs_data->chunk_size);
	}

	//start calculate mbr/msr parameters
	if (ncfs_data->disk_raid_type == 1000) {	//EMBR
		ncfs_data->mbr_n = ncfs_data->disk_total_num;
		ncfs_data->mbr_k = ncfs_data->data_disk_num;	//currently only support n=k-1 or n=k-2
		n = ncfs_data->mbr_n;
		k = ncfs_data->mbr_k;
		d = n - 1;
		ncfs_data->mbr_m = k * d - (int)(k * (k - 1) / 2);
		ncfs_data->mbr_c =
		    (int)(n * (n - 1) / 2) - (int)(k * (2 * n - k - 1) / 2);
		ncfs_data->mbr_segment_size =
		    (int)(2 * (ncfs_data->mbr_m + ncfs_data->mbr_c) / n);
		ncfs_data->segment_size = ncfs_data->mbr_segment_size;

		field_power = 8;
		ncfs_data->generator_matrix =
		    reed_sol_vandermonde_coding_matrix(ncfs_data->mbr_m,
						       ncfs_data->mbr_c,
						       field_power);

		printf
		    ("MBR parameters: n=%d, k=%d, m=%d, c=%d, segment_size=%d\n",
		     ncfs_data->mbr_n, ncfs_data->mbr_k, ncfs_data->mbr_m,
		     ncfs_data->mbr_c, ncfs_data->mbr_segment_size);
	} else if (ncfs_data->disk_raid_type == 3000) {	//Reed-Solomon code
		ncfs_data->segment_size = 1;
		//generate the last m rows of the distribution matrix in GF(2^w) for reed solomon
		ncfs_data->generator_matrix =
		    reed_sol_vandermonde_coding_matrix(ncfs_data->data_disk_num,
						       ncfs_data->
						       disk_total_num -
						       ncfs_data->data_disk_num,
						       8);
	} else {
		ncfs_data->segment_size = 1;
	}

	fileSystemLayer->get_disk_status(ncfs_data);	

	fileSystemLayer->get_operation_mode(ncfs_data);	

	//test print
	printf
	    ("***main: disk_total_num=%d, data_disk_num=%d, chunk_size=%d, raid_type=%d,space_list_num=%d, no_cache=%d, no_gui=%d\n",
	     ncfs_data->disk_total_num, ncfs_data->data_disk_num,
	     ncfs_data->chunk_size, ncfs_data->disk_raid_type,
	     ncfs_data->space_list_num, ncfs_data->no_cache, ncfs_data->no_gui);
	for (j = 0; j < ncfs_data->disk_total_num; j++) {
		printf("***main: j=%d, dev=%s, free_offset=%d, free_size=%d\n",
		       j, ncfs_data->dev_name[j], ncfs_data->free_offset[j],
		       ncfs_data->free_size[j]);
	}

	for (; i < argc; i++)
		argv[i] = argv[i + 1];
	argc--;

	fprintf(stderr, "about to call fuse_main\n");
	//initialize gobal objects
	NCFS_DATA = ncfs_data;
	cacheLayer = new CacheLayer(ncfs_data);
	storageLayer = new StorageLayer();

	reportLayer = new ReportLayer();
	if (ncfs_data->no_gui == 0) {
		diskusageLayer = new DiskusageReport(100);
		processReport = new ProcessReport();
	}
	fileSystemLayer->codingLayer = new CodingLayer();

	//--Wilson Edit Start
	//cacheLayer->DiskCreate(ncfs_data);
	fuse_stat = fuse_main(argc, argv, &ncfs_oper, ncfs_data);
	for (i = 0; i < FILE_MAX; ++i) {
		cacheLayer->FileInfoDestroy(i);
	}
	fprintf(stderr, "fuse_main returned %d\n", fuse_stat);
	//--Wilson Edit End

	return fuse_stat;
}
