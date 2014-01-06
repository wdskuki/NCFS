#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "cache.hh"
#include "disk.hh"

struct ecfs_state* ECFS_DATA;
CacheLayer* cacheLayer; 
DiskLayer* diskLayer;

int main(int argc, char* argv[])
{
	char dev_0[32] = "/dev/loop0";
	struct ecfs_state *ecfs_data;
	int block_size;
	int disk_id;
	long long size;
	long long offset;
	char *buf;
	char symbol;
	int devfd;
	FILE *outfile;
	FILE *checkfile;
	int validity;
	char old_char, new_char;
	int i, j;
	int test_block_total;
	int option;	//1 for write; 2 for read; 3 for both
	
	ecfs_data = (struct ecfs_state *) calloc(sizeof(struct ecfs_state), 1);

	// make sure it's run as the root
	if (geteuid() != 0) {
		fprintf(stderr, "ERR: %s not run as a root\n", argv[0]);
		exit(-1);
	}

	if (argc < 2){
		option = 1;	//default value
	}
	else if (strcmp(argv[1],"write\0") == 0){
		option = 1;
	}
	else if (strcmp(argv[1],"read\0") == 0){
		option = 2;
	}
	else if (strcmp(argv[1],"both\0") == 0){
		option = 3;
	}

	if (argc < 3){
		test_block_total = 100;	//default value
	}
	else{
		test_block_total = atoi(argv[2]);
	}

	printf("option = %d; test_block_total = %d\n", option, test_block_total);

	//Define values of ecfs_data
	ecfs_data->disk_total_num = 1;
	ecfs_data->chunk_size = 4096;
	block_size = ecfs_data->chunk_size;
	ecfs_data->dev_name = (char **)calloc(ecfs_data->disk_total_num, sizeof(char *));
	ecfs_data->dev_name[0] = dev_0;
	ECFS_DATA = ecfs_data;

	//****** Start testing ******

	// initialize the layers
	cacheLayer = new CacheLayer(ecfs_data);
	diskLayer = new DiskLayer(); 
	
	disk_id = 0;
	size = block_size;
	buf = (char*)malloc(sizeof(char)*ecfs_data->chunk_size);

	if ((option == 1) || (option == 3)){
		//Cache Write Start
		printf("\nWriting data blocks to device\n");
		for (i=0; i < test_block_total; i++){
			symbol = 'A' + i%26;
			for (j=0; j < block_size; j++){
				buf[j] = symbol;
			}

			offset = i * block_size;	
			cacheLayer->DiskWrite(disk_id,buf,size,offset);
		}
		//Cache Write End

		//Validate blocks on device 
		printf("\nChecking blocks on device\n");
		validity = 0;
		devfd = open("/dev/loop0",O_RDONLY);
		old_char = 'Z';               
		for (i=0; i < test_block_total; i++){    
			offset = i * block_size;     
			lseek(devfd, offset, SEEK_SET);
			read(devfd, &new_char, 1);
			if (((int)(new_char)-(int)(old_char) == 1) || 
					((old_char == 'Z') && (new_char == 'A'))){
				old_char = new_char;
			}
			else{
				validity = i;
				break;                 
			}
		}
		close(devfd);

		if (validity == 0){
			printf("\nBlocks in device are in correct sequence\n");
		}
		else{
			printf("ERROR: Blocks in device are NOT in correct sequence"); 
			printf(" at block number: %d\n",i);
		}                              
	}

	if ((option == 2) || (option ==3)){
		//Cache Read Start
		printf("\nWriting blocks read to output file\n");
		outfile = fopen("test_output.dat","w+");
		for (i=0; i < test_block_total; i++){
			offset = i * block_size;
			cacheLayer->DiskRead(disk_id,buf,size,offset);
			fprintf(outfile,"%s",buf);
		}
		fclose(outfile);
		//Cache Read End

		//Validate output file
		printf("\nChecking output file\n");
		validity = 0;
		checkfile = fopen("test_output.dat","r");
		old_char = 'Z';               
		for (i=0; i < test_block_total; i++){    
			offset = i * block_size;     
			fseek(checkfile, offset, SEEK_SET);
			new_char = fgetc(checkfile);
			if (((int)(new_char)-(int)(old_char) == 1) || 
					((old_char == 'Z') && (new_char == 'A'))){
				old_char = new_char;
			}
			else{
				validity = i;
				break;                 
			}
		}
		fclose(checkfile);

		if (validity == 0){
			printf("\nBlocks in output file are in correct sequence\n");
		}
		else{
			printf("ERROR: Blocks in output file are NOT in correct sequence"); 
			printf(" at block number: %d\n",i);
		}                              
	}

	cacheLayer->DiskDestroy();

	//End testing

	return 0;
}
