#include "testCoding.hh"

//#include "recovery.hh"
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


int main(int argc, char const *args[])
{

	int n, k, d;
	int field_power;
	int j;
	

	// if(argc!=3){
	//   printf("Usage: ./recover (new device name) (failed node id)\n");
	//   abort();
	// }
	
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

//	fileSystemLayer->get_raid_metadata(ncfs_data);




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

	fileSystemLayer->print_device_setting_withInput(ncfs_data);

	for (j=0; j<ncfs_data->disk_total_num; j++){
		printf("***main: j=%d, dev=%s, free_offset=%d, free_size=%d\n",
				j,ncfs_data->dev_name[j], ncfs_data->free_offset[j],ncfs_data->free_size[j]);
	}

	return 0;





/********************************

// 	//initialize gobal objects
// 	NCFS_DATA = ncfs_data;

// //	recovery_init();
	    
// 	memset(_newdevice,0,PATH_MAX);
// 	strncpy(_newdevice,args[1],strlen(args[1]));
// 	_failed_disk_id=atoi(args[2]);

// 	//set status of the failed disk to be 1 
// 	fileSystemLayer->set_device_status(_failed_disk_id,1);

// 	printf("Recover: -%s -%d\n",_newdevice,_failed_disk_id);
	
// //	RecoveryTool::recover();

// 	return 1;

******************************/
}


