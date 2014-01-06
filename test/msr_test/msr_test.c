//************************
// MSR test program
// Write data in MSR encoding; and then perform recovery (degraded read) for different cases.
// Modify the DEFINE constant value to set number of failed disks, their IDs, and values of data
//
// Author: Michael Yu (cmyu@cse.cuhk.edu.hk)
// Date: 17 December 2010
//
// Reference: Yuchong's MSR presentation.
//
//************************


#include <stdio.h>
#include <stdlib.h>

#define BLOCKSIZE 1
#define DISKNUM 6
#define FIELDSIZE 8

#define DISK_FAILED_NUM 2
#define FIRST_FAIL_ID 2
#define SECOND_FAIL_ID 5

#define TEST_DATA_BEGIN 65

#define MSR_N 6
#define MSR_K 3
#define MSR_M 9
//M=k(n-k)
#define MSR_C 9
//C=M
#define MSR_SEGMENT_SIZE 3
//segment size per disk
//segmnet size = (M+C)/N
#define MSR_SEGMENT_NUM 3

//for (n,k)=(6,3):
//disk_num=6, mbr_n=6, mbr_k=3, mbr_m=9, mbr_c=9, mbr_segment_size=3


static unsigned short *gflog, *gfilog; 
//polynomial = x^8 + x^4 + x^3 + x^2 + 1
static unsigned int prim_poly_8 = 0x11d;

static int msr_coding_matrix[MSR_C][MSR_M];

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
                sum = sum % field_max;

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

                if (diff < 0){
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

        result = 1 << value;

	return result;
}


//MSR generate coding matrix
// currently hardcode for (6,3,5) MSR code
void msr_gen_coding_matrix(int msr_n, int msr_k, int msr_m, int msr_c)
{
	//M'0 coding coefficients
	msr_coding_matrix[0][0] = 3;
        msr_coding_matrix[0][1] = 0;
        msr_coding_matrix[0][2] = 0;
        msr_coding_matrix[0][3] = 3;
        msr_coding_matrix[0][4] = 0;
        msr_coding_matrix[0][5] = 0;
        msr_coding_matrix[0][6] = 3;
        msr_coding_matrix[0][7] = 0;
        msr_coding_matrix[0][8] = 0;

        //M'1 coding coefficients
        msr_coding_matrix[1][0] = 2;  
        msr_coding_matrix[1][1] = 1;
        msr_coding_matrix[1][2] = 0;
        msr_coding_matrix[1][3] = 3;
        msr_coding_matrix[1][4] = 1;
        msr_coding_matrix[1][5] = 0;
        msr_coding_matrix[1][6] = 1;  
        msr_coding_matrix[1][7] = 1;
        msr_coding_matrix[1][8] = 0;

        //M'2 coding coefficients
        msr_coding_matrix[2][0] = 2;  
        msr_coding_matrix[2][1] = 0;
        msr_coding_matrix[2][2] = 1;
        msr_coding_matrix[2][3] = 1;
        msr_coding_matrix[2][4] = 0;
        msr_coding_matrix[2][5] = 1;
        msr_coding_matrix[2][6] = 3;  
        msr_coding_matrix[2][7] = 0;
        msr_coding_matrix[2][8] = 1;

        //M'3 coding coefficients
        msr_coding_matrix[3][0] = 1;  
        msr_coding_matrix[3][1] = 2;
        msr_coding_matrix[3][2] = 0;
        msr_coding_matrix[3][3] = 2;
        msr_coding_matrix[3][4] = 2;
        msr_coding_matrix[3][5] = 0;
        msr_coding_matrix[3][6] = 3;  
        msr_coding_matrix[3][7] = 2;
        msr_coding_matrix[3][8] = 0;

        //M'4 coding coefficients
        msr_coding_matrix[4][0] = 0;  
        msr_coding_matrix[4][1] = 3;
        msr_coding_matrix[4][2] = 0;
        msr_coding_matrix[4][3] = 0;
        msr_coding_matrix[4][4] = 1;
        msr_coding_matrix[4][5] = 0;
        msr_coding_matrix[4][6] = 0;  
        msr_coding_matrix[4][7] = 2;
        msr_coding_matrix[4][8] = 0;

        //M'5 coding coefficients
        msr_coding_matrix[5][0] = 0;  
        msr_coding_matrix[5][1] = 2;
        msr_coding_matrix[5][2] = 1;
        msr_coding_matrix[5][3] = 0;
        msr_coding_matrix[5][4] = 1;
        msr_coding_matrix[5][5] = 2;
        msr_coding_matrix[5][6] = 0;  
        msr_coding_matrix[5][7] = 3;
        msr_coding_matrix[5][8] = 3;

        //M'6 coding coefficients
        msr_coding_matrix[6][0] = 1;  
        msr_coding_matrix[6][1] = 0;
        msr_coding_matrix[6][2] = 2;
        msr_coding_matrix[6][3] = 3;
        msr_coding_matrix[6][4] = 0;
        msr_coding_matrix[6][5] = 2;
        msr_coding_matrix[6][6] = 2;  
        msr_coding_matrix[6][7] = 0;
        msr_coding_matrix[6][8] = 2;

        //M'7 coding coefficients
        msr_coding_matrix[7][0] = 0;  
        msr_coding_matrix[7][1] = 1;
        msr_coding_matrix[7][2] = 2;
        msr_coding_matrix[7][3] = 0;
        msr_coding_matrix[7][4] = 3;
        msr_coding_matrix[7][5] = 3;
        msr_coding_matrix[7][6] = 0;  
        msr_coding_matrix[7][7] = 2;
        msr_coding_matrix[7][8] = 1;

        //M'8 coding coefficients
        msr_coding_matrix[8][0] = 0;  
        msr_coding_matrix[8][1] = 0;
        msr_coding_matrix[8][2] = 3;
        msr_coding_matrix[8][3] = 0;
        msr_coding_matrix[8][4] = 0;
        msr_coding_matrix[8][5] = 2;
        msr_coding_matrix[8][6] = 0;  
        msr_coding_matrix[8][7] = 0;
        msr_coding_matrix[8][8] = 2;

	return;
}


//MSR find block id for a block based on its disk id and block no
int msr_find_block_id(int disk_id, int block_no, int msr_segment_size, int msr_n, int msr_k)
{
	int msr_block_id;
	int block_offset;

	block_offset = block_no % msr_segment_size;

	if (disk_id >= msr_k){
		msr_block_id = -1;
	}
	else{
		msr_block_id = disk_id * msr_segment_size + block_offset;
	}

	return msr_block_id;
}


//MSR find block id for a encoded block based on its disk id and block no
int msr_find_code_block_id(int disk_id, int block_no, int msr_segment_size, int msr_n, int msr_k)
{
        int msr_block_id;
        int block_offset;

        block_offset = block_no % msr_segment_size;

        if (disk_id < msr_k){       
                msr_block_id = -1;
        }
        else{
                msr_block_id = (disk_id - msr_k) * msr_segment_size + block_offset;
        }        

        return msr_block_id;
}



//MSR get disk id for a block based on its MSR block number
int msr_get_disk_id(int msr_block_id, int msr_segment_size, int msr_n, int msr_k)
{
	int disk_id;

	disk_id = (int)(msr_block_id / msr_segment_size);

	return disk_id;
}


//MSR get disk number for a block based on its disk id and MSR block number
int msr_get_block_no(int msr_block_id, int msr_segment_size)
{
	int block_no;

	block_no = msr_block_id % msr_segment_size;

	return block_no;
}



//MSR get encoded disk id for a block based on its MSR block number
int msr_get_code_disk_id(int msr_block_id, int msr_segment_size, int msr_n, int msr_k)
{
        int disk_id;

        disk_id = (int)(msr_block_id / msr_segment_size);
	disk_id = disk_id + msr_k;

        return disk_id;
}


//MSR get encoded block number for a block based on its MSR block number
int msr_get_code_block_no(int msr_block_id, int msr_segment_size)
{
        int block_no;

        block_no = msr_block_id % msr_segment_size;

        return block_no;
}



int msr_find_single_decode_block_id(int msr_block_id, int msr_segment_size)
{
	int decode_block_id;

	decode_block_id = (msr_block_id % msr_segment_size) * (msr_segment_size + 1);

	return decode_block_id;
}


//test msr algorithm
int main(int argc, char *argv[])
{
	char write_data[MSR_M * MSR_SEGMENT_NUM][BLOCKSIZE];
	char disk_data[DISKNUM][MSR_SEGMENT_SIZE * MSR_SEGMENT_NUM][BLOCKSIZE];
	char read_data[MSR_M * MSR_SEGMENT_NUM][BLOCKSIZE];
	char temp_buf[BLOCKSIZE];
	int i, j, k, p;
	int temp;
	char temp_char;

	int msr_segment_id;
	int msr_block_id;
	int disk_id, block_no;
	int code_disk_id, code_block_no;
	int temp_msr_block_id, temp_disk_id, temp_block_no;
	char encoded_data;
	int coefficient;
	int decode_block_id, decode_disk_id, decode_block_no;

        int disk_failed_num = DISK_FAILED_NUM;
        int disk_first_id = FIRST_FAIL_ID;
        int disk_second_id = SECOND_FAIL_ID;
        int disk_another_failed_id;            

	//Generate GF table
	gen_tables(8);

	//Generate coding matrix
	msr_gen_coding_matrix(MSR_N, MSR_K, MSR_M, MSR_C);

	//Print the coding matrix
	printf("\nCoding matrix\n");
	for (i=0; i<MSR_C; i++){
		printf("M'%d = ",i);
		for (j=0; j<MSR_M; j++){
			temp = msr_coding_matrix[i][j];
			if (temp != 0){
				if (temp == 1){
					printf("M%d + ",j);
				}
				else{
					printf("%dM%d + ",temp,j);
				}
			}
		}
		printf("\n");
	}
	printf("\n");


	//Generate write buffer
	for (i=0; i<MSR_M * MSR_SEGMENT_NUM; i++){
		temp_char = TEST_DATA_BEGIN + i;
		for (j=0; j<BLOCKSIZE; j++){
			write_data[i][j] = temp_char;
		}
	}

	//PRINT Write Buffer
	printf("\n******PRINT Write Buffer******\n");
	for (i=0; i <MSR_M * MSR_SEGMENT_NUM; i++){
		printf("%d Data block: ",i);
		for (j=0; j<BLOCKSIZE; j++){
			printf("%c(%d) ",write_data[i][j],write_data[i][j]); 
		}
		printf("\n");
	}


	//Nullify disk data
        for (i=0; i <DISKNUM; i++){
                for (j=0; j<MSR_SEGMENT_SIZE * MSR_SEGMENT_NUM; j++){
                        for (k=0; k<BLOCKSIZE; k++){
                                disk_data[i][j][k] = 0;
                        }
                }           
        }

	//Write data to disk
        printf("\n******Writing data to disk******\n");
	
	for (i=0; i<MSR_M * MSR_SEGMENT_NUM; i++){
		//find data block location
		msr_segment_id = (int)(i / MSR_M);  //find msr_segment_id;
		msr_block_id = i % MSR_M;
		disk_id = msr_get_disk_id(msr_block_id, MSR_SEGMENT_SIZE, MSR_N, MSR_K);
		block_no = msr_get_block_no(msr_block_id, MSR_SEGMENT_SIZE);
		block_no = block_no + msr_segment_id * MSR_SEGMENT_SIZE;
                printf("M%d disk_id=%d block_no=%d\n",msr_block_id,disk_id,block_no);

		//write data block
		for (k=0; k<BLOCKSIZE; k++){
			disk_data[disk_id][block_no][k] = write_data[msr_block_id + msr_segment_id * MSR_M][k];
		}

		//find coded block location
		for (p=0; p<MSR_C; p++){
		   	if (msr_coding_matrix[p][msr_block_id] != 0){

				code_disk_id = msr_get_code_disk_id(p, MSR_SEGMENT_SIZE, MSR_N, MSR_K);
				code_block_no = msr_get_code_block_no(p, MSR_SEGMENT_SIZE);
				code_block_no = code_block_no + msr_segment_id * MSR_SEGMENT_SIZE;

				//generate encoded data
				for (k=0; k<BLOCKSIZE; k++){
					temp_buf[k] = 0;
				}

				for (j=0; j<MSR_M; j++){
					coefficient = msr_coding_matrix[p][j];
					if (coefficient != 0){
						temp_msr_block_id = j;
						temp_disk_id = msr_get_disk_id(temp_msr_block_id, MSR_SEGMENT_SIZE, 
										MSR_N, MSR_K);
						temp_block_no = msr_get_block_no(temp_msr_block_id, MSR_SEGMENT_SIZE);
						temp_block_no = temp_block_no + msr_segment_id * MSR_SEGMENT_SIZE;

						for (k=0; k<BLOCKSIZE; k++){
							temp_char = disk_data[temp_disk_id][temp_block_no][k];
							encoded_data = (char)gf_mul((unsigned char)coefficient, 
									(unsigned char)temp_char,  FIELDSIZE);
							temp_buf[k] = temp_buf[k] ^ encoded_data;
						}
					}
				}

				//write coded block
                		for (k=0; k<BLOCKSIZE; k++){
                        		disk_data[code_disk_id][code_block_no][k] = temp_buf[k];
                		}
			}
		}
	}

        //PRINT Disk Data
        printf("\n******PRINT Disk Data******\n");
        for (i=0; i <DISKNUM; i++){
                printf("###Disk id: %d\n",i);
                for (j=0; j<MSR_SEGMENT_SIZE * MSR_SEGMENT_NUM; j++){
			msr_block_id = msr_find_block_id(i,j,MSR_SEGMENT_SIZE,MSR_N,MSR_K);
			if (msr_block_id == -1){
				msr_block_id = msr_find_code_block_id(i,j,MSR_SEGMENT_SIZE,MSR_N,MSR_K);
				msr_block_id = msr_block_id + 100;
			}
                        printf("##Relative block number: %d. MSR ID: {%d}, Data: ",j,msr_block_id); 
			for (k=0; k<BLOCKSIZE; k++){
				printf("%c(%d) ",disk_data[i][j][k],disk_data[i][j][k]);
			}
			printf("\n");
                }           
                printf("\n");
        }


        //Read data from disk to read buffer        

	for (i=0; i<MSR_M * MSR_SEGMENT_NUM; i++){

		msr_block_id = i % MSR_M;
		msr_segment_id = i / MSR_M;
                disk_id = msr_get_disk_id(msr_block_id, MSR_SEGMENT_SIZE, MSR_N, MSR_K);
                block_no = msr_get_block_no(msr_block_id, MSR_SEGMENT_SIZE);		
		block_no = block_no + msr_segment_id * MSR_SEGMENT_SIZE;				

                disk_another_failed_id = disk_second_id; 
                if (disk_id == disk_first_id){
                        disk_another_failed_id = disk_second_id;              
                }
                else if (disk_id == disk_second_id){
                        disk_another_failed_id = disk_first_id;
                }


                disk_another_failed_id = disk_second_id; 

                if (disk_id == disk_first_id){
                        disk_another_failed_id = disk_second_id;              
                }
                else if (disk_id == disk_second_id){
                        disk_another_failed_id = disk_first_id;
                }

		if ((disk_id != FIRST_FAIL_ID) && (disk_id != SECOND_FAIL_ID)){
		//Case of no disk failure
			printf("\nBlock is accessible\n");
			for (k=0; k<BLOCKSIZE; k++){
				read_data[i][k] = disk_data[disk_id][block_no][k];
			}
		}
		else{
		//Cases of disk failure

                        //Find the single coded block for handling one-node failure
                        decode_block_id = msr_find_single_decode_block_id(msr_block_id, MSR_SEGMENT_SIZE);
                        decode_disk_id = msr_get_code_disk_id(decode_block_id, MSR_SEGMENT_SIZE, MSR_N, MSR_K);
                        decode_block_no = msr_get_code_block_no(decode_block_id, MSR_SEGMENT_SIZE);
                        decode_block_no = decode_block_no + msr_segment_id * MSR_SEGMENT_SIZE;


			if ( (disk_failed_num == 1) ||
				((disk_failed_num == 2) && (disk_another_failed_id >= (int)(MSR_N/2)) && 
				(disk_another_failed_id != decode_disk_id)) ){
			//Case of single disk failure
				printf("\nSingle disk failure. msr_block_id: %d. disk_id: %d. block_no: %d\n",
						msr_block_id, disk_id, block_no);

				for (k=0; k<BLOCKSIZE; k++){
					temp_buf[k] = disk_data[decode_disk_id][decode_block_no][k];
				}

				//Calculate the requested data
				for (j=0; j<MSR_M; j++){
					coefficient = msr_coding_matrix[decode_block_id][j];
					if ((j != msr_block_id) && (coefficient != 0)){
						temp_disk_id = msr_get_disk_id(j, MSR_SEGMENT_SIZE, MSR_N, MSR_K);
						temp_block_no = msr_get_block_no(j, MSR_SEGMENT_SIZE);
						temp_block_no = temp_block_no + msr_segment_id * MSR_SEGMENT_SIZE;
						for (k=0; k<BLOCKSIZE; k++){
							temp_char = (char)gf_mul((unsigned char)coefficient, 
								(unsigned char)disk_data[temp_disk_id][temp_block_no][k], 
								FIELDSIZE);
							temp_buf[k] = temp_buf[k] ^ temp_char;
						}
					}
				}

				for (k=0; k<BLOCKSIZE; k++){
					read_data[i][k] = (char)gf_div((unsigned char)temp_buf[k], 
							(unsigned char)msr_coding_matrix[decode_block_id][msr_block_id], 
							FIELDSIZE);
				}
			}
                        else if ((disk_failed_num == 2) && (disk_another_failed_id == decode_disk_id)){
                                printf("\nTwo-disk failure (D + primary parity). msr_block_id: %d. disk_id: %d. block_no: %d\n",  
                                                msr_block_id, disk_id, block_no);


				//generate block status array
				int block_status[MSR_M];   //0 for healthy; 1 for failed
				int code_block_status[MSR_C];
				int sec_code_block_id, sec_code_disk_id, sec_code_block_no;
				int sec_code_block_coeff[MSR_M];
				char sec_code_block_value[BLOCKSIZE];
				int temp_block_coeff[MSR_M];
				char temp_block_value[BLOCKSIZE];
				int count_failed_blocks;
				int temp_coeff1, temp_coeff2;

				for (j=0; j<MSR_M; j++){
					temp_disk_id = msr_get_disk_id(j, MSR_SEGMENT_SIZE, MSR_N, MSR_K);
					if ((temp_disk_id == disk_id) || (temp_disk_id == disk_another_failed_id)){
						block_status[j] = 1;
					}
					else{
						block_status[j] = 0;
					}
				}

				for (j=0; j<MSR_C; j++){
                                        temp_disk_id = msr_get_code_disk_id(j, MSR_SEGMENT_SIZE, MSR_N, MSR_K);
                                        if ((temp_disk_id == disk_id) || (temp_disk_id == disk_another_failed_id)){
                                                code_block_status[j] = 1;
                                        }
                                        else{
                                                code_block_status[j] = 0;
                                        }
				}

				//get secondary decode block
				sec_code_block_id = 0;
				for (j=0; j<MSR_C; j++){
					count_failed_blocks = MSR_M;
					//find code block having target block and fewest failed blocks
					if ((code_block_status[j] == 0) && (msr_coding_matrix[j][msr_block_id] != 0)){
						temp = 0;
						for (k=0; k<MSR_M; k++){
							if ((k != msr_block_id) && (msr_coding_matrix[j][k] != 0)
							&& (block_status[k] == 1)){
								temp++;
							}
						}
						if (temp < count_failed_blocks){
							sec_code_block_id = j;
							count_failed_blocks = temp;
						}						
					}
				}
				sec_code_disk_id = msr_get_code_disk_id(sec_code_block_id, MSR_SEGMENT_SIZE, MSR_N, MSR_K);
                        	sec_code_block_no = msr_get_code_block_no(sec_code_block_id, MSR_SEGMENT_SIZE);
                        	sec_code_block_no = sec_code_block_no + msr_segment_id * MSR_SEGMENT_SIZE;
				printf("\n***get sec_decode_block. block_id=%d, sec_code_block_id=%d\n",
										msr_block_id,sec_code_block_id);

				//generate temporary coefficient array
				for (k=0; k<MSR_M; k++){
					sec_code_block_coeff[k] = msr_coding_matrix[sec_code_block_id][k];
				}

				for (k=0; k<BLOCKSIZE; k++){
					sec_code_block_value[k] = disk_data[sec_code_disk_id][sec_code_block_no][k];
				}
				
				//eliminate non-target values (failed blocks)
				for (j=0; j<MSR_M; j++){
					if ((j != msr_block_id) && (sec_code_block_coeff[j] != 0)
					&& (block_status[j] != 0)){
printf("\n****eliminate %d\n",j);
						//find decode block, eliminate the failed block
						temp_msr_block_id = msr_find_single_decode_block_id(j, MSR_SEGMENT_SIZE);
						temp_disk_id = msr_get_code_disk_id(temp_msr_block_id, 
									MSR_SEGMENT_SIZE, MSR_N, MSR_K);
						temp_block_no = msr_get_code_block_no(temp_msr_block_id, MSR_SEGMENT_SIZE);
						temp_block_no = temp_block_no + msr_segment_id * MSR_SEGMENT_SIZE;
printf("****code bock id: %d\n",temp_msr_block_id);
						for (k=0; k<MSR_M; k++){
							temp_block_coeff[k] = msr_coding_matrix[temp_msr_block_id][k];
						}

						if ((temp_disk_id != disk_id) || (temp_disk_id != disk_another_failed_id)){
							for (k=0; k<BLOCKSIZE; k++){
								temp_block_value[k] = disk_data[temp_disk_id][temp_block_no][k];
							}
						}else{
							printf("\nError: Attempt to access failed disk\n");
						}

						temp_coeff1 = sec_code_block_coeff[j];
						temp_coeff2 = temp_block_coeff[j];

printf("\n");
						for (k=0; k<MSR_M; k++){
							//temp = temp_coeff1 * temp_block_coeff[k]
							//	^ temp_coeff2 * sec_code_block_coeff[k];

							temp = (int)((char)gf_mul((unsigned char)temp_coeff1, 
									(unsigned char)temp_block_coeff[k], FIELDSIZE)
								^ (char)gf_mul((unsigned char)temp_coeff2,
									(unsigned char)sec_code_block_coeff[k], FIELDSIZE));

							sec_code_block_coeff[k] = temp;
printf("%d:%d ",k,temp);
						}
printf("\n");

						for (k=0; k<BLOCKSIZE; k++){
							sec_code_block_value[k] = 
								(char)gf_mul((unsigned char)sec_code_block_value[k],
										(unsigned char)temp_coeff2, FIELDSIZE);
                                                        temp_char = (char)gf_mul((unsigned char)temp_block_value[k], 
                                                                          (unsigned char)temp_coeff1, FIELDSIZE);
							sec_code_block_value[k] ^= temp_char;
						}
					}
				}

				//eliminate non-target values (healthy blocks)
				for (j=0; j < MSR_M; j++){
					if ((j != msr_block_id) && (sec_code_block_coeff[j] != 0)){
						temp_msr_block_id = j;
						temp_disk_id = msr_get_disk_id(temp_msr_block_id, MSR_SEGMENT_SIZE, MSR_N, MSR_K);
						temp_block_no = msr_get_code_block_no(temp_msr_block_id, MSR_SEGMENT_SIZE);
						temp_block_no = temp_block_no + msr_segment_id * MSR_SEGMENT_SIZE;

						for (k=0; k<BLOCKSIZE; k++){
							temp_block_value[k] = disk_data[temp_disk_id][temp_block_no][k];
						}

						temp = sec_code_block_coeff[temp_msr_block_id];
						for (k=0; k<BLOCKSIZE; k++){
							temp_char = (char)gf_mul((unsigned char)temp,
								(unsigned char)temp_block_value[k], FIELDSIZE);
							sec_code_block_value[k] ^= temp_char;
						}
						sec_code_block_coeff[temp_msr_block_id] = 0;
					}
				}


				//find target value
				temp = sec_code_block_coeff[msr_block_id];
				for (k=0; k<BLOCKSIZE; k++){
                                        read_data[i][k] = (char)gf_div((unsigned char)sec_code_block_value[k], 
                                                        (unsigned char)temp, FIELDSIZE);
				}
                        }
			else{
                                printf("\nTwo data-disk failure, or more than two disk failure. Non-supported failure."); 
				printf("msr_block_id: %d. disk_id: %d. block_no: %d\n",
                                                msr_block_id, disk_id, block_no);
			}
		}
	}


        //PRINT Read Buffer
        printf("\n******PRINT Read Buffer******\n");
        for (i=0; i <MSR_M * MSR_SEGMENT_NUM; i++){
                printf("%d data block: ",i);
                for (j=0; j<BLOCKSIZE; j++){
                        printf("%c(%d) ",read_data[i][j],read_data[i][j]); 
                }
                printf("\n");
        }

	return 0;
}
