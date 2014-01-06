#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#define TOTAL_DISK_NUM 1
#define CACHE_SIZE 5    //cache size in MB
#define DISK_SIZE 2048  //size of each node
#define BLOCK_SIZE 4096 //block size in Bytes
#define FRACTION_OF_DATA_REQUIRED 8   //how much fraction of data is required

int main(int argc, char *argv[])
{
	int i;
	long long size, offset;
	char *buf;
	char *buf2;
	int fd[4];
	long long counter;
	long long counter2;
	long long cache_space;
	long long read_count;
	long long data_count;
	int retstat;

        struct timeval t1, t2;
        double duration;

	size = BLOCK_SIZE;
	buf = (char*)malloc(size*sizeof(char)); 
        buf2 = (char*)malloc(size*sizeof(char)); 
	counter = 0;
	read_count = 0;
	data_count = 0;
	cache_space = CACHE_SIZE * 1024 * 1024;

        gettimeofday(&t1,NULL);

	while (counter + CACHE_SIZE <= DISK_SIZE){
        	fd[0] = open("/dev/loop1",O_RDONLY);
        	//fd[1] = open("/dev/loop2",O_RDONLY);
        	//fd[2] = open("/dev/loop3",O_RDONLY);
        	//fd[3] = open("/dev/loop4",O_RDONLY);

		counter2 = 0;
		while (counter2 + BLOCK_SIZE <= cache_space){
			offset = counter*1024*1024 + counter2;
			for (i=0; i < TOTAL_DISK_NUM; i++){
				retstat = pread(fd[i],buf,size,offset);
				memcpy(buf2,buf,size);
				read_count++;
				data_count = data_count + retstat;
			}
			counter2 = counter2 + 
				BLOCK_SIZE * FRACTION_OF_DATA_REQUIRED;
		}

		for (i=0; i < TOTAL_DISK_NUM; i++){
			close(fd[i]);
		}
		counter = counter + CACHE_SIZE;
	}

        gettimeofday(&t2,NULL);
        duration = t2.tv_sec - t1.tv_sec + 
        	(t2.tv_usec - t1.tv_usec)/1000000.0;

	printf("Elapsed time = %f\n",duration);
	printf("Read count = %Ld\n",read_count);
	printf("Data count = %Ld\n",data_count);

        free(buf);
        free(buf2);

	return 0;
}
