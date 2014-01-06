//************************
// MBR test program
// Write data in MBR encoding; and then perform recovery (degraded read) for different cases.
// Modify the DEFINE constant value to set number of failed disks, their IDs, and values of data
//
// Author: Michael Yu (cmyu@cse.cuhk.edu.hk)
// Date: 26 November 2010
//
// Reference: Yuchong's MBR implementation programs and pseudo code.
//
//************************


#include <stdio.h>
#include <stdlib.h>

#define BLOCKSIZE 1
#define DISKNUM 5
#define FIELDSIZE 8

#define DISK_FAILED_NUM 2
#define FIRST_FAIL_ID 0
#define SECOND_FAIL_ID 1

#define TEST_DATA_BEGIN 65

#define MBR_N 5
#define MBR_K 3
#define MBR_M 9
#define MBR_M_CODEBLOCK 1
#define MBR_SEGMENT_SIZE 4
#define MBR_SEGMENT_NUM 2
//segment size per disk
//segmnet size = (M+1)x2/N = (9+1)x2/5 = 4 

//for (n,k)=(5,3):
//disk_num=5, mbr_n=5, mbr_k=3, mbr_m=9, mbr_m_codeblock=1, mbr_segment_size=4

//for (n,k)=(6,4):
//disk_num=6, mbr_n=6, mbr_k=4, mbr_m=14, mbr_m_codeblock=1, mbr_segment_size=5

//for (n,k)=(3,2):
//disk_num=3, mbr_n=3, mbr_k=2, mbr_m=2, mbr_m_codeblock=1, mbr_segment_size=2

//for (n,k)=(4,2):
//disk_num=4, mbr_n=4, mbr_k=2, mbr_m=5, mbr_m_codeblock=1, mbr_segment_size=3

//for (n,k)=(5,2):
//disk_num=5, mbr_n=5, mbr_k=2, mbr_m=7, mbr_m_codeblock=3, mbr_segment_size=4


static unsigned short *gflog, *gfilog; 
//polynomial = x^8 + x^4 + x^3 + x^2 + 1
static unsigned int prim_poly_8 = 0x11d;

//*****************************
// This routine is used to create logarithm and 
// inverse logarithm tables for computing P and Q parity.
// In our P and Q computation, we use all 1s for the coefficients of P.
//*****************************
int gen_tables(int s)
{
	unsigned int b, index, gf_elements;
	
	//convert s to the number of elements for the table
	gf_elements = 1 << s;
	gflog = (unsigned short*) malloc(sizeof(unsigned short)*gf_elements);
	gfilog = (unsigned short*) malloc(sizeof(unsigned short)*gf_elements);
	b = 1;

	for (index=0; index < gf_elements-1; index++){
		gflog[b] = (unsigned char)index;
		gfilog[index] = (unsigned char)b;
		b <<= 1;
		if (b & gf_elements){
			b ^= prim_poly_8;
		}
	}

	return 0;
}


//Multiply a and b.
//Finite field size is 2 power s.
//Return the finite field result.
unsigned short gf_mul(int a, int b, int s)
{
	unsigned int field_max;
	int sum;
	unsigned short result;

	if ((a == 0) || (b == 0)){
		result = 0;
	}
	else{
		field_max = (1 << s) - 1;
		sum = gflog[a] + gflog[b];

		if (sum > field_max){
			sum = (sum % (field_max));
		}

		result = gfilog[sum];  
	}

	return(result);
}


//Divide a by b.
//Finite field size is 2 power s.
//Return the finite field result.
unsigned short gf_div(int a, int b, int s)
{
        unsigned int field_max;
        int diff;
        unsigned short result;

	if (a == 0){
		result = 0;
	}
	else{
        	field_max = (1 << s) - 1;

        	diff = gflog[a] - gflog[b];

		//printf("\n a=%d, b=%d, diff=%d\n",a,b,diff);
		if (diff < 0){
			//diff = (diff % field_max) - 1;
			diff = diff + field_max;
		}

        	result = gfilog[diff];
	}

        return(result);
}


//Get coefficient g
int get_coefficient(int value)
{
	int result;

	if (value == 0)
		result = 1;
	else if (value == 1)
		result = 4;
	else if (value == 2)
		result = 8;
	else if (value == 3)
		result = 16;
	else if (value == 4)
		result = 29;

	return result;
}


//MBR find block id for a block based on its disk id and block no
int mbr_find_block_id(int disk_id, int block_no, int mbr_segment_size)
{
	int mbr_block_id;
	int block_offset, mbr_offset;
	int counter, i;

	block_offset = block_no % mbr_segment_size;

	if (block_offset < disk_id){
		mbr_block_id = -1;
	}
	else{
		mbr_offset = block_offset - disk_id;
		mbr_block_id = 0;
		counter = mbr_segment_size;
		for (i=0; i<disk_id; i++){
			mbr_block_id = mbr_block_id + counter;
			counter--;
		}

		mbr_block_id = mbr_block_id + mbr_offset;
	}

	return mbr_block_id;
}


//MBR find block id for a duplicated block based on its disk id and block no
int mbr_find_dup_block_id(int disk_id, int block_no, int mbr_segment_size)
{
        int mbr_block_id;
        int block_offset, mbr_offset;
        int counter, i;

        block_offset = block_no % mbr_segment_size;

        if (block_offset >= disk_id){
                mbr_block_id = -1;
        }
        else{
                mbr_offset = disk_id - 1;
                mbr_block_id = 0;
                counter = mbr_segment_size;
                for (i=0; i<block_offset; i++){
                        mbr_block_id = mbr_block_id + counter;
                        counter--;
			mbr_offset--;
                }

                mbr_block_id = mbr_block_id + mbr_offset;
        }

        return mbr_block_id;
}



//MBR get disk id for a block based on its MBR block number
int mbr_get_disk_id(int mbr_block_id, int mbr_segment_size)
{
	int mbr_block_group, counter, temp;

        mbr_block_group = 0;
        counter = mbr_segment_size;
        temp = (mbr_block_id + 1) - counter;

        while (temp > 0){
        	counter--;
                temp = temp - counter;
                (mbr_block_group)++;
        }

	return mbr_block_group;
}


//MBR get disk number for a block based on its disk id and MBR block number
int mbr_get_block_no(int disk_id, int mbr_block_id, int mbr_segment_size)
{
	int block_no;
	int mbr_offset, j;

        mbr_offset = mbr_block_id;
        for (j=0; j<disk_id; j++){
        	mbr_offset = mbr_offset - (mbr_segment_size - j);
        }
        block_no = mbr_offset + disk_id;

	return block_no;
}



//MBR get duplicated disk id for a block based on its MBR block number
int mbr_get_dup_disk_id(int mbr_block_id, int mbr_segment_size)
{
	int dup_disk_id;
	int disk_id, mbr_offset;
	int j;

        disk_id = mbr_get_disk_id(mbr_block_id, mbr_segment_size);
        mbr_offset = mbr_block_id;
        for (j=0; j<disk_id; j++){
        	mbr_offset = mbr_offset - (mbr_segment_size - j);
        }
	
        dup_disk_id = mbr_offset + 1 + disk_id;

	return dup_disk_id;
}


//MBR get duplicated block number for a block based on its MBR block number
int mbr_get_dup_block_no(int disk_id, int mbr_block_id, int mbr_segment_size)
{
	int dup_block_no;

	dup_block_no = disk_id;

	return dup_block_no;
}


//test mbr algorithm
int main(int argc, char *argv[])
{
	char write_data[MBR_M * MBR_SEGMENT_NUM][BLOCKSIZE];
	char disk_data[DISKNUM][MBR_SEGMENT_SIZE * MBR_SEGMENT_NUM][BLOCKSIZE];
	char read_data[MBR_M * MBR_SEGMENT_NUM][BLOCKSIZE];
	char temp_buf[BLOCKSIZE];
	char temp_char;
	int i, j, k, p;
	int temp;

	int mbr_segment_id;
	int disk_failed_num;
	int mbr_block_id;
	int disk_id, block_no;
	int dup_disk_id, dup_block_no;
	int temp_mbr_block_id, temp_disk_id, temp_block_no;


	//Initialize parameters
	disk_failed_num = DISK_FAILED_NUM;

	//Generate write buffer
	for (i=0; i<MBR_M * MBR_SEGMENT_NUM; i++){
		temp_char = TEST_DATA_BEGIN + i;
		for (j=0; j<BLOCKSIZE; j++){
			write_data[i][j] = temp_char;
		}
	}

	//PRINT Write Buffer
	printf("\n******PRINT Write Buffer******\n");
	for (i=0; i <MBR_M * MBR_SEGMENT_NUM; i++){
		printf("M%d data: ",i);
		for (j=0; j<BLOCKSIZE; j++){
			printf("%c(%d) ",write_data[i][j],write_data[i][j]); 
		}
		printf("\n");
	}


	//Nullify disk data
        for (i=0; i <DISKNUM; i++){
                for (j=0; j<MBR_SEGMENT_SIZE * MBR_SEGMENT_NUM; j++){
                        for (k=0; k<BLOCKSIZE; k++){
                                disk_data[i][j][k] = 0;
                        }
                }           
        }


	//Write data to disk
        printf("\n******Writing data to disk******\n");
	
	for (i=0; i<MBR_M * MBR_SEGMENT_NUM; i++){
		//find data block location
		mbr_segment_id = (int)(i / MBR_M);  //find mbr_segment_id;
		//mbr_block_id = i;
		mbr_block_id = i % MBR_M;
		disk_id = mbr_get_disk_id(mbr_block_id, MBR_SEGMENT_SIZE);
		block_no = mbr_get_block_no(disk_id, mbr_block_id, MBR_SEGMENT_SIZE);
		block_no = block_no + mbr_segment_id * MBR_SEGMENT_SIZE;
                printf("M%d disk_id=%d block_no=%d\n",mbr_block_id,disk_id,block_no);

		//write data block
		for (k=0; k<BLOCKSIZE; k++){
			disk_data[disk_id][block_no][k] = write_data[mbr_block_id + mbr_segment_id * MBR_M][k];
		}

		//find duplicated block location
		//dup_disk_id = mbr_offset + 1 + disk_id;
		dup_disk_id = mbr_get_dup_disk_id(mbr_block_id, MBR_SEGMENT_SIZE);
		dup_block_no = mbr_get_dup_block_no(disk_id, mbr_block_id, MBR_SEGMENT_SIZE);
		dup_block_no = dup_block_no + mbr_segment_id * MBR_SEGMENT_SIZE;
                printf("M%d dup_disk_id=%d dup_block_no=%d\n\n",mbr_block_id,dup_disk_id,dup_block_no);

		//write duplicated block
                for (k=0; k<BLOCKSIZE; k++){
                        disk_data[dup_disk_id][dup_block_no][k] = write_data[mbr_block_id + mbr_segment_id * MBR_M][k];
                }

		//find coded block location, and write coded block(s)
		for (p=0; p<MBR_M_CODEBLOCK; p++){
			mbr_block_id = MBR_M + p;

			disk_id = mbr_get_disk_id(mbr_block_id, MBR_SEGMENT_SIZE);
			block_no = mbr_get_block_no(disk_id, mbr_block_id, MBR_SEGMENT_SIZE);
			block_no = block_no + mbr_segment_id * MBR_SEGMENT_SIZE;
	
            printf("M%d disk_id=%d block_no=%d\n",mbr_block_id,disk_id,block_no);

			//Important: Generate coded block data. Replace this with suitable encoding method.
			for (k=0; k<BLOCKSIZE; k++){
				temp_buf[k] = 0;
			}

			for (j=0; j<MBR_M; j++){
				temp_mbr_block_id = j;
				temp_disk_id = mbr_get_disk_id(temp_mbr_block_id, MBR_SEGMENT_SIZE);
				temp_block_no = mbr_get_block_no(temp_disk_id, temp_mbr_block_id, MBR_SEGMENT_SIZE);
				temp_block_no = temp_block_no + mbr_segment_id * MBR_SEGMENT_SIZE;

				for (k=0; k<BLOCKSIZE; k++){
					temp_buf[k] = temp_buf[k] ^ disk_data[temp_disk_id][temp_block_no][k];
				}
			}

                	//write coded block
                	for (k=0; k<BLOCKSIZE; k++){
                        	disk_data[disk_id][block_no][k] = temp_buf[k];
                	}

                	//find duplicated coded block location
                	//dup_disk_id = mbr_offset + 1 + disk_id;
                	dup_disk_id = mbr_get_dup_disk_id(mbr_block_id, MBR_SEGMENT_SIZE);
                	dup_block_no = mbr_get_dup_block_no(disk_id, mbr_block_id, MBR_SEGMENT_SIZE);
					dup_block_no = dup_block_no + mbr_segment_id * MBR_SEGMENT_SIZE;

                	printf("M%d dup_disk_id=%d dup_block_no=%d\n\n",mbr_block_id,dup_disk_id,dup_block_no);

                	//write duplicated block
                	for (k=0; k<BLOCKSIZE; k++){
                        	disk_data[dup_disk_id][dup_block_no][k] = temp_buf[k];
                	}
		}
	}


        //PRINT Disk Data
        printf("\n******PRINT Disk Data******\n");
        for (i=0; i <DISKNUM; i++){
                printf("###Disk id: %d\n",i);
                for (j=0; j<MBR_SEGMENT_SIZE * MBR_SEGMENT_NUM; j++){
			mbr_block_id = mbr_find_block_id(i,j,MBR_SEGMENT_SIZE);
			if (mbr_block_id == -1){
				mbr_block_id = mbr_find_dup_block_id(i,j,MBR_SEGMENT_SIZE);
				mbr_block_id = mbr_block_id + 100;
			}
                        printf("##Relative block number: %d. MBR ID: {%d}, Data: ",j,mbr_block_id); 
			for (k=0; k<BLOCKSIZE; k++){
				printf("%c(%d) ",disk_data[i][j][k],disk_data[i][j][k]);
			}
			printf("\n");
                }           
                printf("\n");
        }


        //Read data from disk to read buffer        

	for (i=0; i<MBR_M * MBR_SEGMENT_NUM; i++){

                //mbr_block_id = i;
				mbr_block_id = i % MBR_M;
				mbr_segment_id = i / MBR_M;
                disk_id = mbr_get_disk_id(mbr_block_id, MBR_SEGMENT_SIZE);
                block_no = mbr_get_block_no(disk_id, mbr_block_id, MBR_SEGMENT_SIZE);		
				block_no = block_no + mbr_segment_id * MBR_SEGMENT_SIZE;				

		if ((disk_id != FIRST_FAIL_ID) && (disk_id != SECOND_FAIL_ID)){
		//Case of no disk failure
			printf("\nBlock is accessible\n");
			for (k=0; k<BLOCKSIZE; k++){
				read_data[i][k] = disk_data[disk_id][block_no][k];
			}
		}
		else{
		//Cases of disk failure
			if (disk_failed_num == 1){
			//Case of single disk failure
			//Find the duplicated block to substitute the failed block

				printf("\nSingle disk failure. mbr_block_id: %d. disk_id: %d. block_no: %d\n",
						mbr_block_id, disk_id, block_no);
                                dup_disk_id = mbr_get_dup_disk_id(mbr_block_id, MBR_SEGMENT_SIZE);
                                dup_block_no = mbr_get_dup_block_no(disk_id, mbr_block_id, MBR_SEGMENT_SIZE);
								dup_block_no = dup_block_no + mbr_segment_id * MBR_SEGMENT_SIZE;

                                for (k=0; k<BLOCKSIZE; k++){
                                        read_data[i][k] = disk_data[dup_disk_id][dup_block_no][k];
                                }
			}

			if (disk_failed_num == 2){

                                printf("\nTwo-disk failure. mbr_block_id: %d. disk_id: %d. block_no: %d\n",
                                                mbr_block_id, disk_id, block_no);
                                dup_disk_id = mbr_get_dup_disk_id(mbr_block_id, MBR_SEGMENT_SIZE);
                                dup_block_no = mbr_get_dup_block_no(disk_id, mbr_block_id, MBR_SEGMENT_SIZE);
								dup_block_no = dup_block_no + mbr_segment_id * MBR_SEGMENT_SIZE;

				if ((dup_disk_id != FIRST_FAIL_ID) && (dup_disk_id != SECOND_FAIL_ID)){
					//Read duplicated block
					for (k=0; k<BLOCKSIZE; k++){
						read_data[i][k] = disk_data[dup_disk_id][dup_block_no][k];
					}
				}
				else{
					//Both blocks failed. Get help from encoded block

					for (k=0; k<BLOCKSIZE; k++){
						temp_buf[k] = 0;
					}

					for (j=0; j<MBR_M + MBR_M_CODEBLOCK; j++){
						//This decoding only applies to 1 encoded block (C=1)
						//and use {1,1,...,1} encoding.

						if (j != mbr_block_id){
							temp_mbr_block_id = j;
							temp_disk_id = mbr_get_disk_id(temp_mbr_block_id, 
											MBR_SEGMENT_SIZE);
							temp_block_no = mbr_get_block_no(temp_disk_id,
									temp_mbr_block_id,MBR_SEGMENT_SIZE);
							temp_block_no = temp_block_no + mbr_segment_id * MBR_SEGMENT_SIZE;

							if (temp_disk_id == -1){
								temp = temp_disk_id;
                                                		temp_disk_id = mbr_get_dup_disk_id(temp_mbr_block_id, 
										MBR_SEGMENT_SIZE);
                                                		temp_block_no = mbr_get_dup_block_no(temp,
										temp_mbr_block_id, MBR_SEGMENT_SIZE);
							}

							if (temp_disk_id != -1){
								for (k=0; k<BLOCKSIZE; k++){
									temp_buf[k] = temp_buf[k] ^ 
									disk_data[temp_disk_id][temp_block_no][k];
								}
							}
						}
					}

					for (k=0; k<BLOCKSIZE; k++){
						read_data[i][k] = temp_buf[k];
					}
				}
			}
		}
	}


        //PRINT Read Buffer
        printf("\n******PRINT Read Buffer******\n");
        for (i=0; i <MBR_M * MBR_SEGMENT_NUM; i++){
                printf("M%d data: ",i);
                for (j=0; j<BLOCKSIZE; j++){
                        printf("%c(%d) ",read_data[i][j],read_data[i][j]); 
                }
                printf("\n");
        }

	return 0;
}
