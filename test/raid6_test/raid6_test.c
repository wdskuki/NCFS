//************************
// RAID6 test program
// Write data in RAID 6 encoding; and then perform recovery for different cases.
// Modify the DEFINE constant value to set number of failed disks, their IDs, and values of data
//
// Author: Michael Yu (cmyu@cse.cuhk.edu.hk)
// Date: 15 November 2010
//
// Reference: Intel. Intelligent RAID 6 Theory: Overview and Implementation. White paper.
//
//************************


#include <stdio.h>
#include <stdlib.h>

#define BLOCKSIZE 1
#define DISKNUM 7
#define FIELDSIZE 8

#define DISK_FAILED_NUM 2
#define FIRST_FAIL_ID 0
#define SECOND_FAIL_ID 4

#define TEST_DATA_BEGIN 0

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


//test raid6 algorithm
int main(int argc, char *argv[])
{

	char disk_data[DISKNUM][BLOCKSIZE];
	char read_data[DISKNUM-2][BLOCKSIZE];
	int disk_id;
	char temp_char;
	int i, j;
	int code_disk_id, parity_disk_id;
	int data_disk_coeff;

	int disk_failed_no = DISK_FAILED_NUM;
	int disk_first_id = FIRST_FAIL_ID;
	int disk_second_id = SECOND_FAIL_ID;
	int disk_another_failed_id;	

	int g1, g2, g12;
	char P_temp[BLOCKSIZE], Q_temp[BLOCKSIZE];

	gen_tables(8);

	printf("\nPrint gflog(x)\n");
	for (i=0; i<256; i++){
		printf("%d(%d) ",i,gflog[i]);
	}
	printf("\n");

        printf("\nPrint gfilog(x)\n");
        for (i=0; i<256; i++){
                printf("%d(%d) ",i,gfilog[i]);
        }
        printf("\n");


	for (i=0; i<DISKNUM; i++){
		if (i < DISKNUM-2){
			temp_char = TEST_DATA_BEGIN + i;
		}
		else{
			temp_char = 0;
		}

		for (j=0; j<BLOCKSIZE; j++){
			disk_data[i][j] = temp_char;
		}
	}

	printf("\n****** PRINT DISK DATA ******\n");
        for (i=0; i<DISKNUM; i++){
                printf("Disk id: %d\n",i);
                for (j=0; j<BLOCKSIZE; j++){
                        printf("%c(%d) ",disk_data[i][j],disk_data[i][j]);
                }
                printf("\n");
        }


	//write P and Q

	code_disk_id = DISKNUM -1;
	parity_disk_id = DISKNUM - 2;

	for (i=0; i<DISKNUM; i++){
		if ( (i != parity_disk_id) && (i != code_disk_id) ){
			
			for (j=0; j<BLOCKSIZE; j++){

				//calculate P
				disk_data[parity_disk_id][j] = 
					disk_data[parity_disk_id][j] ^ disk_data[i][j];
			

                                //calculate the coefficient of the data block
                                data_disk_coeff = i;

                                if (i > code_disk_id){   
                                        (data_disk_coeff)--;                  
                                }
                                if (i > parity_disk_id){
                                        (data_disk_coeff)--;
                                }        
                                data_disk_coeff = DISKNUM - 3 - data_disk_coeff;
                                data_disk_coeff = get_coefficient(data_disk_coeff);

                                printf("\ndata disk coefficient = %d\n",data_disk_coeff);

				//calculate Q
				temp_char = disk_data[code_disk_id][j];
				disk_data[code_disk_id][j] = temp_char ^ 
					(char)gf_mul((unsigned char)disk_data[i][j],data_disk_coeff,FIELDSIZE);

				printf(" newQ=%c(%d) \n",disk_data[code_disk_id][j],
					(unsigned int)disk_data[code_disk_id][j]);
			}

		}
	}


        printf("\n****** PRINT DISK DATA ******\n");
        for (i=0; i<DISKNUM; i++){
                printf("Disk id: %d\n",i);
                for (j=0; j<BLOCKSIZE; j++){
                        printf("%c(%d) ",disk_data[i][j],disk_data[i][j]);
                }
                printf("\n");
        }


	//read data

	for (i=0; i<DISKNUM-2; i++){
		for (j=0; j<BLOCKSIZE; j++){
			read_data[i][j] = 0;
		}
	}


	for (disk_id=0; disk_id<DISKNUM-2; disk_id++){

		disk_another_failed_id = disk_second_id;

		if (disk_id == disk_first_id){
			disk_another_failed_id = disk_second_id;
		}
		else if (disk_id == disk_second_id){
			disk_another_failed_id = disk_first_id;
		}

		printf("\nDisk ID: %d, another failed disk ID: %d\n",disk_id, disk_another_failed_id);

		//case of the disk is not failed
		if ( (disk_failed_no == 0) || 
				((disk_id != disk_first_id) && (disk_id != disk_second_id)) ){
			printf("NO DISK FAIL\n");
			for (j=0; j<BLOCKSIZE; j++){
				read_data[disk_id][j] = disk_data[disk_id][j];
			}
		}
		//cases of single disk fail
		else if ( (disk_failed_no == 1) ||
		((disk_failed_no == 2) && (disk_another_failed_id == code_disk_id)) ){
			printf("One disk fail, or 1 data plus Q disk fail\n");
			for (i = 0; i < DISKNUM; i++){
				if ( (i != disk_id) && (i != code_disk_id) ){
					for (j=0; j<BLOCKSIZE; j++){
						read_data[disk_id][j] =
							read_data[disk_id][j] ^ disk_data[i][j];
					}
				}
			}
		}
		else if (disk_failed_no == 2){
			if (disk_another_failed_id == parity_disk_id){
				printf("1 data plus P disk fail\n");

				//calculate Q'
				for (i=0; i<DISKNUM; i++){
                                        if ( ((i != disk_first_id) && (i != disk_second_id)) 
					&& (i != parity_disk_id) && (i != code_disk_id) ){
                                                for (j=0; j < BLOCKSIZE; j++){

                                			//calculate the coefficient of the data block
                                			data_disk_coeff = i;

                                			if (i > code_disk_id){
                                        			(data_disk_coeff)--;
                                			}
                                			if (i > parity_disk_id){
                                        			(data_disk_coeff)--;
                                			}
                                			data_disk_coeff = DISKNUM - 3 - data_disk_coeff;              
                                			data_disk_coeff = get_coefficient(data_disk_coeff);

                                			printf("\ndata disk coefficient = %d\n",data_disk_coeff);       

                                                        temp_char = read_data[disk_id][j];
                                                        read_data[disk_id][j] = read_data[disk_id][j] ^
								(char)gf_mul((unsigned char)disk_data[i][j],data_disk_coeff,FIELDSIZE);
                                                }
                                        }

                                	for (j=0; j<BLOCKSIZE; j++){
                                        	printf("Q'=%c(%d) ",read_data[disk_id][j],read_data[disk_id][j]);
                                	}

				}


                                //calculate Q xor Q'
                                for (j=0; j < BLOCKSIZE; j++){
                                	read_data[disk_id][j] = read_data[disk_id][j] ^ disk_data[code_disk_id][j];
                                }
        
                                for (j=0; j<BLOCKSIZE; j++){
                                        printf("Q'+Q=%c(%d) ",read_data[disk_id][j],read_data[disk_id][j]);
                                }

                                //calculate the coefficient of the data block
                                data_disk_coeff = disk_id;

                                if (disk_id > code_disk_id){
                                        (data_disk_coeff)--;
                                }
                                if (disk_id > parity_disk_id){
                                        (data_disk_coeff)--;
                                }
                                data_disk_coeff = DISKNUM - 3 - data_disk_coeff;
				data_disk_coeff = get_coefficient(data_disk_coeff);

				printf("\ndata disk coefficient = %d\n",data_disk_coeff);

                                //decode the origianl data block
                                for (j=0; j < BLOCKSIZE; j++){
					temp_char = read_data[disk_id][j];
                                        read_data[disk_id][j] = (char)gf_div((unsigned char)temp_char,data_disk_coeff,FIELDSIZE);
                                }
			}
			else{
			//case of two data disk fail
				printf("2 data disk fail\n");

				//calculate g1
				g1 = disk_id;
                                if (g1 > code_disk_id){
                                        (g1)--;
                                }
                                if (g1 > parity_disk_id){
                                        (g1)--;
                                }
                                g1 = DISKNUM - 3 - g1;
                                g1 = get_coefficient(g1);
				printf("g1=%d \n",g1);

				//calculate g2
                                g2 = disk_another_failed_id;
                                if (g2 > code_disk_id){     
                                        (g2)--;             
                                }
                                if (g2 > parity_disk_id){     
                                        (g2)--;             
                                }
                                g2 = DISKNUM - 3 - g2;                          
                                g2 = get_coefficient(g2);   
				printf("g2=%d \n",g2);

				//calculate g12
				g12 = g1 ^ g2;

				//calculate P'
				for (j=0; j<BLOCKSIZE; j++){
					P_temp[j] = 0;
				}

				for (i=0; i<DISKNUM; i++){
					if ( (i != disk_first_id) && (i != disk_second_id) &&
						(i != parity_disk_id) && (i != code_disk_id) )
					{
						for (j=0; j<BLOCKSIZE; j++){
							P_temp[j] = P_temp[j] ^ disk_data[i][j];
						}
					}
				}

				for (j=0; j<BLOCKSIZE; j++){
					P_temp[j] = P_temp[j] ^ disk_data[parity_disk_id][j];
					//P_temp = P' xor P
				}


				//calculate Q'
				for (j=0; j<BLOCKSIZE; j++){
					Q_temp[j] = 0;
				}

                                for (i=0; i<DISKNUM; i++){
                                        if ( ((i != disk_first_id) && (i != disk_second_id)) 
                                        && (i != parity_disk_id) && (i != code_disk_id) ){

                                                //calculate the coefficient of the data block
                                                data_disk_coeff = i;

                                                if (i > code_disk_id){
                                                        (data_disk_coeff)--;
                                                }
                                                if (i > parity_disk_id){
                                                        (data_disk_coeff)--;
                                                }
                                                data_disk_coeff = DISKNUM - 3 - data_disk_coeff;              
                                                data_disk_coeff = get_coefficient(data_disk_coeff);

                                                printf("\ndata disk coefficient = %d\n",data_disk_coeff);       

                                                for (j=0; j < BLOCKSIZE; j++){
                                                        temp_char = Q_temp[j];
                                                        Q_temp[j] = temp_char ^ 
							(char)gf_mul((unsigned char)disk_data[i][j],data_disk_coeff,FIELDSIZE);
                                                }
                                        }

                                        for (j=0; j<BLOCKSIZE; j++){
                                                printf("Q'=%c(%d) ",Q_temp[j],Q_temp[j]);
                                        }
                                }

				//calculate D
				for (j=0; j<BLOCKSIZE; j++){
					temp_char = (char)(gf_mul(g2,(unsigned char)P_temp[j],FIELDSIZE) 
								^ Q_temp[j] ^ disk_data[code_disk_id][j]);
					read_data[disk_id][j] = (char)gf_div((unsigned char)temp_char, g12, FIELDSIZE);
				}
			}
		}
	}


	//print data read
	printf("\n****** PRINT READ DATA ******\n");
        for (i=0; i<DISKNUM-2; i++){
                printf("Read disk id: %d\n",i);
                for (j=0; j<BLOCKSIZE; j++){
                        printf("%c(%d) ",read_data[i][j],read_data[i][j]);
                }
                printf("\n");
        }

	return 0;
}
