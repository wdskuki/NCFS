#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

int main(int argc, char *argv[])
{
	int uid = getuid();
	if (uid != 0) {
		printf("Please run as root\n");
		printf
		    ("Usage: benchmark [-h help] [-v verify] [-k keep] [-t tempdir] [-d targetdir] [-b blocksize] [-s testsize(MB)]\n");
		return 0;
	}
	int testsize = 1024;
	int blocksize = 4096;
	char tempdir[256] = "/tmp";
	char testdir[256] = ".";
	int verify = 0;
	int keep = 0;
	int ch;
	opterr = 0;
	while ((ch = getopt(argc, argv, "hvkt:d:b:s:")) != -1)
		switch (ch) {
		case 'h':
			printf
			    ("Usage: benchmark [-h help] [-v verify] [-k keep] [-t tempdir] [-d targetdir] [-b blocksize] [-s testsize(MB)]\n");
			return 0;
			break;
		case 'k':
			keep = 1;
			break;
		case 't':
			strncpy(tempdir, optarg, 255);
			strcat(tempdir, "\0");
			break;
		case 'd':
			strncpy(testdir, optarg, 255);
			strcat(testdir, "\0");
			break;
		case 'b':
			blocksize = atoi(optarg);
			break;
		case 's':
			testsize = atoi(optarg);
			break;
		case 'v':
			verify = 1;
			break;
		default:
			printf("Incorrect Argument\n");
		}
	printf("//////////Test Setup\\\\\\\\\\\\\\\\\\\\\n");
	printf("Test Size %dMB\n", testsize);
	printf("Block Size %d\n", blocksize);
	printf("Target Device %s\n", testdir);
	printf("Temp Device %s\n", tempdir);
	printf("\\\\\\\\\\\\\\\\\\\\Test Setup//////////\n");

	int i, j, k;
	srand(10);
	char verifyfilename[256] = { '\0' };

	int numofblock = 1024 * 1024 / blocksize;
	int blocksizeby4 = blocksize / 4;
	int *randomcontent = (int *)malloc(sizeof(int) * blocksizeby4);
	int verifyfile;

	struct timeval endtime;
	struct timeval starttime;

	char testfilename[256] = { '\0' };
	sprintf(testfilename, "%s/%d", testdir, rand());
	printf("Test file name = %s\n", testfilename);

	printf("Write to Test File, with block size %d\n", blocksize);
	gettimeofday(&starttime, NULL);
	int testfile = open(testfilename, O_WRONLY | O_CREAT | O_SYNC);
	//randomfile = open(filename,O_RDONLY);
	for (i = 0; i < testsize; i++) {
		for (j = 0; j < (numofblock); j++) {
			memset(randomcontent, 0, blocksize);

			for (k = 0; k < blocksizeby4; k++) {
				randomcontent[k] = rand();
			}
			printf("Block %d, %d\n", j + i * numofblock,
			       randomcontent[0]);

			write(testfile, randomcontent, blocksize);
		}
	}
	fsync(testfile);
	close(testfile);
	gettimeofday(&endtime, NULL);
	double duration =
	    endtime.tv_sec - starttime.tv_sec + (endtime.tv_usec -
						 starttime.tv_usec) / 1000000.0;
	printf("Duration = %fs\n", duration);
	printf("Target Device Write Speed = %fMB/s\n",
	       (float)testsize / (float)duration);

	sync();
	testfile = open(testfilename, O_RDONLY | O_RSYNC | O_SYNC);
	printf("%d Read from Test File\n", testfile);
	gettimeofday(&starttime, NULL);
	for (i = 0; i < testsize; i++) {
		for (j = 0; j < (numofblock); j++) {
			memset(randomcontent, 0, blocksize);
			read(testfile, randomcontent, blocksize);
		}
	}
	close(testfile);
	gettimeofday(&endtime, NULL);
	duration =
	    endtime.tv_sec - starttime.tv_sec + (endtime.tv_usec -
						 starttime.tv_usec) / 1000000.0;
	printf("Duration = %fs\n", duration);
	printf("Target Device Read Speed = %fMB/s\n",
	       (float)testsize / (float)duration);

	if (verify == 1) {
		sprintf(verifyfilename, "%s/%d", tempdir, rand());
		srand(10);
		//rand();
		printf("%d\n", rand());
		printf("Write Verify File\n");
		verifyfile = open(verifyfilename, O_WRONLY | O_CREAT);
		for (i = 0; i < testsize; i++) {
			for (j = 0; j < (numofblock); j++) {
				memset(randomcontent, 0, blocksize);

				for (k = 0; k < blocksizeby4; k++) {
					randomcontent[k] = rand();
				}

				write(verifyfile, randomcontent, blocksize);
			}
		}
		close(verifyfile);
		printf("Verify File created, Diff going\n");
		int *randomcontent2 = (int *)malloc(sizeof(int) * blocksizeby4);
		verifyfile = open(verifyfilename, O_RDONLY);
		testfile = open(testfilename, O_RDONLY | O_SYNC);
		for (i = 0; i < testsize; i++) {
			for (j = 0; j < (numofblock); j++) {
				memset(randomcontent, 0, blocksize);
				memset(randomcontent2, 0, blocksize);
				int ret =
				    read(testfile, randomcontent, blocksize);
				int ret2 =
				    read(verifyfile, randomcontent2, blocksize);
				int ret3 =
				    memcmp(randomcontent2, randomcontent,
					   blocksize);
				if (ret3 != 0) {
					printf("%d %d Block %d differ\n", ret,
					       ret2, j + i * numofblock);
					printf("%d %d\n", randomcontent[0],
					       randomcontent2[0]);
				}
			}
		}
		close(verifyfile);
		close(testfile);
		if (keep == 0) {
			unlink(verifyfilename);
			printf("Remove Verify File %s\n", verifyfilename);
		}
	}
	if (keep == 0) {
		unlink(testfilename);
		printf("Remove Test File %s\n", testfilename);
	}
	return 0;
}
