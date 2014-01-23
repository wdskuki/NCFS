#include "recovery.hh"
#include <sys/time.h>
#include <ctime>

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>

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

#include "../filesystem/filesystem_common.hh"
#include "../filesystem/filesystem_utils.hh"
#include "../cache/cache.hh"
#include "../coding/coding.hh"
#include "../storage/storage.hh"
#include "../gui/report.hh"
#include "../gui/diskusage_report.hh"
#include "../gui/process_report.hh"

//Add by zhuyunfeng on October 11, 2011 begin.
#include "../config/config.hh"
//Add by zhuyunfeng on October 11, 2011 end.

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

char _newdevice[PATH_MAX];
int _failed_disk_id;


/*
 * Common Recover Method
 */
int ConventionalRecover(int fail_disk_id){
	  
	    NCFS_DATA->process_state = 1;
	    int __recoversize = NCFS_DATA->disk_size[fail_disk_id];

	    struct timeval starttime, endtime;
	    double duration;
	    double data_size;

	    gettimeofday(&starttime,NULL);
	    NCFS_DATA->dev_name[fail_disk_id] = (char*)realloc(NCFS_DATA->dev_name[fail_disk_id],strlen(_newdevice));

	    memset(NCFS_DATA->dev_name[fail_disk_id],0,strlen(_newdevice)+1);
	    strncpy(NCFS_DATA->dev_name[fail_disk_id],_newdevice,strlen(_newdevice));
	    cacheLayer->DiskCacheName(fail_disk_id,_newdevice);

	    //storageLayer->disk_fd[fail_disk_id] = open(NCFS_DATA->dev_name[fail_disk_id],O_RDWR);
	    //storageLayer->DiskRenew(fail_disk_id);

	    for(int i = 0; i < __recoversize; ++i){

	    	printf("i = %d\n", i);

		    int block_size = NCFS_DATA->chunk_size;
		    int offset = i * block_size;
		    //(bug@@)char *buf = (char*)calloc(block_size,0);
		    char buf[block_size];
		    int retstat = fileSystemLayer->codingLayer->decode(fail_disk_id,buf,block_size,offset);

		    retstat = cacheLayer->DiskWrite(fail_disk_id,buf,block_size,offset);

	    }
	    
	    //NCFS_DATA->disk_status[fail_disk_id] = 0;
	    fileSystemLayer->set_device_status(fail_disk_id,0);

	    gettimeofday(&endtime,NULL);

	    fprintf(stderr,"\n\n\nRecovery on disk %d, new device %s Done.\n\n\n",fail_disk_id,_newdevice);

	    duration = endtime.tv_sec - starttime.tv_sec + (endtime.tv_usec-starttime.tv_usec)/1000000.0;
	    data_size  = __recoversize * (NCFS_DATA->chunk_size) / (1024 * 1024);
	    
	    //printf("Elapsed Time = %fs\n", duration);
	    printf("Elapsed Time = %f\n", duration);
	    printf("Repair Throughput = %f MB/s\n", (float)(data_size / duration));
	    printf("Storage Node Size = %f MB\n", (float)data_size);
	    printf("Block Size = %d B\n", NCFS_DATA->chunk_size);
			
	    NCFS_DATA->process_state = 0;
	    return 0;
	    
}

//Add by Dongsheng Wei on Jan. 20, 2014 begin.




int mdr_I_recover_one_disk(int fail_disk_id){
	  	
	  	//printf("debug: mdr_I_recover()\n");


	    NCFS_DATA->process_state = 1;
	    int __recoversize = NCFS_DATA->disk_size[fail_disk_id];

	    int strip_size = fileSystemLayer->codingLayer->mdr_I_get_strip_size();
	    int disk_total_num = NCFS_DATA->disk_total_num;
	    int block_size = NCFS_DATA->chunk_size;

	    struct timeval starttime, endtime;
	    double duration;
	    double data_size;

	    gettimeofday(&starttime,NULL);
	    NCFS_DATA->dev_name[fail_disk_id] = (char*)realloc(NCFS_DATA->dev_name[fail_disk_id],strlen(_newdevice));

	    memset(NCFS_DATA->dev_name[fail_disk_id],0,strlen(_newdevice)+1);
	    strncpy(NCFS_DATA->dev_name[fail_disk_id],_newdevice,strlen(_newdevice));
	    cacheLayer->DiskCacheName(fail_disk_id,_newdevice);

	    //storageLayer->disk_fd[fail_disk_id] = open(NCFS_DATA->dev_name[fail_disk_id],O_RDWR);
	    //storageLayer->DiskRenew(fail_disk_id);

	   	char*** pread_stripes;
	   	int r = strip_size / 2;
	    pread_stripes = (char ***)malloc(sizeof(char** ) * r);
	    for(int i = 0; i < r; i++){
	    	pread_stripes[i] = (char **)malloc(sizeof(char* )* disk_total_num);
	    	for(int j = 0; j < disk_total_num; j++){
	    		pread_stripes[i][j] = (char *)malloc(sizeof(char)* block_size);
	    		//memset(pread_stripes[i][j], 0, block_size);
	    	}
	    }

	    // int r = strip_size / 2;
	    // char pread_stripes[r][disk_total_num][block_size];
		
	    for(int i = 0; i < __recoversize; i += strip_size){
		    for(int i = 0; i < r; i++){
		    	for(int j = 0; j < disk_total_num; j++){
		    		memset(pread_stripes[i][j], 0, block_size);
		    	}
		    }

		    long long offset = i * block_size;

		    long long buf_size = strip_size * block_size;
		    char buf[buf_size];

		    int retstat = fileSystemLayer->codingLayer->mdr_I_recover_oneStripeGroup(fail_disk_id, 
		    												buf, buf_size, offset, pread_stripes);
		    retstat = cacheLayer->DiskWrite(fail_disk_id,buf,strip_size*block_size,offset);
	    }
	    
	    for(int i = 0; i < r; i++){
	    	for(int j = 0; j < disk_total_num; j++){
	    		free(pread_stripes[i][j]);
	    	}
	    	free(pread_stripes[i]);
	    }
	    free(pread_stripes);	    


	    //NCFS_DATA->disk_status[fail_disk_id] = 0;
	    fileSystemLayer->set_device_status(fail_disk_id,0);

	    gettimeofday(&endtime,NULL);

	    fprintf(stderr,"\n\n\nRecovery on disk %d, new device %s Done.\n\n\n",fail_disk_id,_newdevice);

	    duration = endtime.tv_sec - starttime.tv_sec + (endtime.tv_usec-starttime.tv_usec)/1000000.0;
	    data_size  = __recoversize * (NCFS_DATA->chunk_size) / (1024 * 1024);
	    
	    //printf("Elapsed Time = %fs\n", duration);
	    printf("Elapsed Time = %f\n", duration);
	    printf("Repair Throughput = %f MB/s\n", (float)(data_size / duration));
	    printf("Storage Node Size = %f MB\n", (float)data_size);
	    printf("Block Size = %d B\n", NCFS_DATA->chunk_size);
			
	    NCFS_DATA->process_state = 0;
	    return 0;
	    
}
//Add by Dongsheng Wei on Jan. 20, 2014 end.

void RecoveryTool::recover(){
	int coding_type = NCFS_DATA->disk_raid_type;
	int disk_total_num = NCFS_DATA->disk_total_num;
	int fail_disk_num = 0;
	int fail_disk_id = 0;
	
	
	for(int i = disk_total_num-1; i >= 0; --i){
		if(NCFS_DATA->disk_status[i] !=0){
			++fail_disk_num;
		}
	}

	fail_disk_id=_failed_disk_id;
	
	if (fail_disk_num > 0){
		if (fail_disk_num == 1){
			//Add by Dongsheng Wei on Jan. 20, 2014 begin.
			if(coding_type == 5000){
				mdr_I_recover_one_disk(fail_disk_id);
			}
			//Add by Dongsheng Wei on Jan. 20, 2014 end.
			else{
				ConventionalRecover(fail_disk_id);
			}
		}
		else if ((fail_disk_num == 2) && (coding_type == 6)){
			ConventionalRecover(fail_disk_id);
		}	
		else if ((coding_type == 1000) &&
				(fail_disk_num <= NCFS_DATA->mbr_n - NCFS_DATA->mbr_k)){
			ConventionalRecover(fail_disk_id);
		}
        else if ((coding_type == 3000) &&
        		(fail_disk_num <= (NCFS_DATA->disk_total_num-NCFS_DATA->data_disk_num))){
            ConventionalRecover(fail_disk_id);
        }
		else{
			printf("Too many disks fail.\n");
		}
	}
	else{
		printf("No disk fails.\n");
	}

}

/*Initialize CacheLayer, StorageLayer and CodingLayer*/
void recovery_init(){
  
	cacheLayer = new CacheLayer(NCFS_DATA);
	storageLayer = new StorageLayer();
        	
	//reportLayer = new ReportLayer();
	if (NCFS_DATA->no_gui == 0){
		diskusageLayer = new DiskusageReport(100);
		processReport = new ProcessReport();
	}
	
	fileSystemLayer->codingLayer = new CodingLayer();
	
}

/*
 * Recovery Utility:
 *                 Use this utility to recover the failed node  by the following command:
 *                 ./recover (new device name) (failed node id)
 */
int main(int argc, char *args[]){
  
	int n, k, d;
	int field_power;
	int j;
	

	if(argc!=3){
	  printf("Usage: ./recover (new device name) (failed node id)\n");
	  abort();
	}
	
	struct ncfs_state *ncfs_data;

	ncfs_data = (struct ncfs_state *) calloc(sizeof(struct ncfs_state), 1);
	if (ncfs_data == NULL) {
		perror("main calloc");
		abort();
	}

        //Initialize the fileSystemLayer and configLayer
	fileSystemLayer = new FileSystemLayer();
	configLayer = new ConfigLayer();

	//initialize those variables for ncfs_data
	ncfs_data->no_cache = 0;
	ncfs_data->no_gui = 0;
	ncfs_data->run_experiment = 0;

	ncfs_data->process_state = 0;
	ncfs_data->encoding_time = 0;
	ncfs_data->decoding_time = 0;
	ncfs_data->diskread_time = 0;
	ncfs_data->diskwrite_time = 0;

	ncfs_data->space_list_num = 0;
	ncfs_data->space_list_head = NULL;

        //read the configure file
	fileSystemLayer->readSystemConfig(ncfs_data);
	
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

	for (j=0; j<ncfs_data->disk_total_num; j++){
		printf("***main: j=%d, dev=%s, free_offset=%d, free_size=%d\n",
				j,ncfs_data->dev_name[j], ncfs_data->free_offset[j],ncfs_data->free_size[j]);
	}

	//initialize gobal objects
	NCFS_DATA = ncfs_data;

	recovery_init();
	    
	memset(_newdevice,0,PATH_MAX);
	strncpy(_newdevice,args[1],strlen(args[1]));
	_failed_disk_id=atoi(args[2]);

	//set status of the failed disk to be 1 
	fileSystemLayer->set_device_status(_failed_disk_id,1);

	printf("Recover: -%s -%d\n",_newdevice,_failed_disk_id);
	
	RecoveryTool::recover();

	return 1;
}
