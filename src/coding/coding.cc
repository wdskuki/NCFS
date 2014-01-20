#include "coding.hh"
#include "../filesystem/filesystem_common.hh"
#include "../filesystem/filesystem_utils.hh"
#include "../cache/cache.hh"
#include "../gui/diskusage_report.hh"

#include <stdio.h>
#include <stdlib.h>
#include <fuse.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>

#include <vector>
#include <map>
#include <set>
#include <iostream>
using namespace std;

extern "C" {
#include "../jerasure/galois.h"
#include "../jerasure/jerasure.h"
#include "../jerasure/reed_sol.h"
}
#define AbnormalError() printf("%s %s %d\n", __FILE__, __func__,__LINE__);
//NCFS context state
extern struct ncfs_state *NCFS_DATA;
extern FileSystemLayer *fileSystemLayer;
extern CacheLayer *cacheLayer;
extern DiskusageReport *diskusageLayer;

/*************************************************************************
 * Private functions
 *************************************************************************/

/*************************************************************************
 * Galios Field functions
 *************************************************************************/

/*
 * gf_gen_tables: Create logarithm and inverse logairthm tables for computing Q parity
 *
 * @param s: field size (2 power s)
 *
 * @return: 0
 */


//Add by Dongsheng Wei on Jan. 17, 2014 begin.
void CodingLayer::print_ivec(vector<int>& ivec){
	int size = ivec.size();
	for(int i = 0; i < size; i++){
		cout<<(ivec.at(i))<<" ";
	}
	cout<<endl;
}

void CodingLayer::print_iivec(vector<vector<int> >& iivec){
	int iivec_size = iivec.size();
	for(int i = 0; i < iivec_size; i++){
		int ivec_size = iivec[i].size();
		cout<<"--";
		for(int j = 0; j < ivec_size; j++){
			cout<<iivec[i][j]<<" ";
		}
		cout<<endl;
	}
	cout<<endl;
}

void CodingLayer::print_ivmap(map<int, vector<vector<int> > >& ivmap, vector<int>& stripeIndexs){
	set<int> iset;
	iset.insert(stripeIndexs.begin(), stripeIndexs.end());

	for(int i = 0; i < strip_size; i++){
		if(iset.find(i) == iset.end()){
			cout<<i<<": ";
			int i_size = ivmap[i].size();
			for(int j = 0; j < i_size; j++){
				int ii_size = ivmap[i][j].size();
				if(ii_size == 0){
					cout<<-1<<"\t";
					continue;
				}
				for(int t = 0; t < ii_size; t++){
					cout<<ivmap[i][j][t]<<" ";
				}
				cout<<"\t";
			}
			cout<<endl;
		}
	}
}
//Add by Dongsheng Wei on Jan. 17, 2014 end.


int CodingLayer::gf_gen_tables(int s)
{
	unsigned int b, index, gf_elements;

	//convert s to the number of elements for the table
	gf_elements = 1 << s;
	this->gflog =
	    (unsigned short *)malloc(sizeof(unsigned short) * gf_elements);
	this->gfilog =
	    (unsigned short *)malloc(sizeof(unsigned short) * gf_elements);
	b = 1;

	for (index = 0; index < gf_elements - 1; index++) {
		this->gflog[b] = (unsigned char)index;
		this->gfilog[index] = (unsigned char)b;
		b <<= 1;
		if (b & gf_elements) {
			b ^= prim_poly_8;
		}
	}

	return 0;
}

/*
 * gf_mul: Multiple a and b 
 *
 * @param a, b: input values for multiplication
 * @param s: field size (2 power s)
 *
 * @return: Product in finite field
 */

unsigned short CodingLayer::gf_mul(int a, int b, int s)
{
	unsigned int field_max;
	int sum;
	unsigned short result;

	if ((a == 0) || (b == 0)) {
		result = 0;
	} else {
		field_max = (1 << s) - 1;
		sum = gflog[a] + gflog[b];
		sum = sum % field_max;

		result = gfilog[sum];
	}

	return (result);
}

/*
 * gf_div: Divide a by b
 *
 * @param a, b: input values for division
 * @param s: field size (2 power s)
 *
 * @return: Divident in finite field
 */

unsigned short CodingLayer::gf_div(int a, int b, int s)
{
	unsigned int field_max;
	int diff;
	unsigned short result;

	if (a == 0) {
		result = 0;
	} else {
		field_max = (1 << s) - 1;

		diff = gflog[a] - gflog[b];

		if (diff < 0) {
			diff = diff + field_max;
		}

		result = gfilog[diff];
	}

	return (result);
}

/*
 * gf_get_coefficient: Get the coefficient of the primitive polynomial
 * polynomial = x^8 + x^4 + x^3 + x^2 + 1
 *
 * @param value: the coefficient offset
 * @param s: field size (2 power s)
 *
 * @return: the coefficient
 */

int CodingLayer::gf_get_coefficient(int value, int s)
{
	int result;

	result = 1 << value;

	return result;
}

/*************************************************************************
 * MBR functions
 *************************************************************************/

/*
 * mbr_find_block_id: find block id for a block based on its disk id and block no
 *
 * @param disk_id: disk ID of the data block
 * @param block_no: block number of the data block
 * @param mbr_segment_size: the segment size for MBR coding
 *
 * @return: mbr block id
 */
int CodingLayer::mbr_find_block_id(int disk_id, int block_no,
				   int mbr_segment_size)
{
	int mbr_block_id;
	int block_offset, mbr_offset;
	int counter, i;

	block_offset = block_no % mbr_segment_size;

	if (block_offset < disk_id) {
		mbr_block_id = -1;
	} else {
		mbr_offset = block_offset - disk_id;
		mbr_block_id = 0;
		counter = mbr_segment_size;
		for (i = 0; i < disk_id; i++) {
			mbr_block_id = mbr_block_id + counter;
			counter--;
		}

		mbr_block_id = mbr_block_id + mbr_offset;
	}

	return mbr_block_id;
}

/*
 * mbr_find_dup_block_id: find block id for a duplicated block based on its disk id and block no
 *
 * @param disk_id: disk ID of the data block
 * @param block_no: block number of the data block
 * @param mbr_segment_size: the segment size for MBR coding
 *
 * @return: mbr block id
 */
int CodingLayer::mbr_find_dup_block_id(int disk_id, int block_no,
				       int mbr_segment_size)
{
	int mbr_block_id;
	int block_offset, mbr_offset;
	int counter, i;

	block_offset = block_no % mbr_segment_size;

	if (block_offset >= disk_id) {
		mbr_block_id = -1;
	} else {
		mbr_offset = disk_id - 1;
		mbr_block_id = 0;
		counter = mbr_segment_size;
		for (i = 0; i < block_offset; i++) {
			mbr_block_id = mbr_block_id + counter;
			counter--;
			mbr_offset--;
		}

		mbr_block_id = mbr_block_id + mbr_offset;
	}

	return mbr_block_id;
}

/*
 * mbr_get_disk_id: get disk id for a block based on its MBR block ID
 *
 * @param mbr_block_id: mbr block ID of the data block
 * @param mbr_segment_size: the segment size for MBR coding
 *
 * @return: disk id
 */
int CodingLayer::mbr_get_disk_id(int mbr_block_id, int mbr_segment_size)
{
	int mbr_block_group, counter, temp;

	mbr_block_group = 0;
	counter = mbr_segment_size;
	temp = (mbr_block_id + 1) - counter;

	while (temp > 0) {
		counter--;
		temp = temp - counter;
		(mbr_block_group)++;
	}

	return mbr_block_group;
}

/*
 * mbr_get_block_no: get block number for a block based on its disk id and MBR block number
 *
 * @param disk id: disk ID of the data block
 * @param mbr_block_id: mbr block ID of the data block
 * @param mbr_segment_size: the segment size for MBR coding
 *
 * @return: block number
 */
int CodingLayer::mbr_get_block_no(int disk_id, int mbr_block_id,
				  int mbr_segment_size)
{
	int block_no;
	int mbr_offset, j;

	mbr_offset = mbr_block_id;
	for (j = 0; j < disk_id; j++) {
		mbr_offset = mbr_offset - (mbr_segment_size - j);
	}
	block_no = mbr_offset + disk_id;

	return block_no;
}

/*
 * mbr_get_dup_disk_id: get disk id for a duplicated block based on its MBR block ID
 *
 * @param mbr_block_id: mbr block ID of the data block
 * @param mbr_segment_size: the segment size for MBR coding
 *
 * @return: disk id
 */
int CodingLayer::mbr_get_dup_disk_id(int mbr_block_id, int mbr_segment_size)
{
	int dup_disk_id;
	int disk_id, mbr_offset;
	int j;

	disk_id = mbr_get_disk_id(mbr_block_id, mbr_segment_size);
	mbr_offset = mbr_block_id;
	for (j = 0; j < disk_id; j++) {
		mbr_offset = mbr_offset - (mbr_segment_size - j);
	}

	dup_disk_id = mbr_offset + 1 + disk_id;

	return dup_disk_id;
}

/*
 * mbr_get_dup_block_no: get block number for a duplicated block based on its disk id and MBR block number
 *
 * @param disk id: disk ID of the original data block
 * @param mbr_block_id: mbr block ID of the data block
 * @param mbr_segment_size: the segment size for MBR coding
 *
 * @return: block number
 */
int CodingLayer::mbr_get_dup_block_no(int disk_id, int mbr_block_id,
				      int mbr_segment_size)
{
	int dup_block_no;

	dup_block_no = disk_id;

	return dup_block_no;
}

//MDR_I operation
//Add by Dongsheng Wei on Jan. 17, 2014 begin.
long long* CodingLayer::mdr_I_iterative_construct_encoding_matrixB(long long *matrix, 
												int k){
	int i, j, t;
	int row = (int)pow(2, k);
	int col = k+1;
	int len = row * col;
	
	int new_k = k+1;
	int new_row = (int)pow(2, new_k);
	int new_col = new_k+1;
	int new_len = new_row * new_col;

	long long * new_matrix = new long long [new_len];
	if(new_k == 1 && matrix == NULL){
		new_matrix[0] = 1;
		new_matrix[1] = 0;
		new_matrix[2] = 0;
		new_matrix[3] = 2;
		return new_matrix;
	}else{
		for(i = 0; i < new_col; i++){
			if(i < new_col-2){
				for(j = 0; j < row; j++){
					new_matrix[j*new_col+i] = (matrix[j*col+i] 
						^ matrix[j*col+col-1]) << row;
				}
				for(j = row; j < new_row; j++){
					new_matrix[j*new_col+i] = (matrix[(j-row)*col+i] 
						^ matrix[(j-row)*col+col-1]);
				}
			}else if(i == new_col-2){
				t = 0;
				for(j = row-1; j >= 0; j--){
					new_matrix[j*new_col+i] = 1 << t;
					t++;
				}
				for(j = row; j < new_row; j++){
					new_matrix[j*new_col+i] = 0;
				}
			}else if(i == new_col-1){
				for(j = 0; j < row; j++){
					new_matrix[j*new_col+i] = 0;
				}			
				for(j = new_row-1; j >= row; j--){
					new_matrix[j*new_col+i] = 1 << t;
					t++;
				}
			}
		}
		//free(matrix);
		delete []matrix;

		return new_matrix;
	}
}

void CodingLayer::mdr_print_matrix(long long* matrix, int row, int col){
	for(int i = 0; i < row; i++){
		for(int j = 0; j < col; j++)
			printf("%lld\t",matrix[i*col+j]);
		printf("\n");
	}
}

long long* CodingLayer::mdr_I_encoding_matrix(int k){ 
	int count;
	long long *mdr_encoding_matrix_B;
	if(k <= 0){
		printf("error: k\n");
		exit(1);
	}
	
	mdr_encoding_matrix_B = new long long [4];

	mdr_encoding_matrix_B[0] = 1;
	mdr_encoding_matrix_B[1] = 0;
	mdr_encoding_matrix_B[2] = 0;
	mdr_encoding_matrix_B[3] = 2;	
	if(k == 1)
		return mdr_encoding_matrix_B;

	count = 1;
	while(count != k){
		mdr_encoding_matrix_B = 
			mdr_I_iterative_construct_encoding_matrixB(
				mdr_encoding_matrix_B, count);
		count++;
	}
	return mdr_encoding_matrix_B;
}

vector<int> CodingLayer::mdr_I_find_q_blocks_id(int disk_id, int block_no){
	vector<int> ivec;

	int row = strip_size;
	int col = NCFS_DATA->data_disk_num + 1;
	int pDisk_id = NCFS_DATA->data_disk_num; 

	int t = 1 << (strip_size - block_no -1);
	for(int i = 0; i < row; i++){
		//data block affect q disk
		if((mdr_I_encoding_matrixB[i*col+disk_id]&t) != 0){
			ivec.push_back(i);
		}

		//parity block (disk_id) affects q disk
		if((mdr_I_encoding_matrixB[i*col+pDisk_id]&t) != 0){
			ivec.push_back(i);
		}		
	}
	return ivec;
}

vector<vector<int> > CodingLayer::mdr_I_repair_qDisk_blocks_id(int block_no){
	vector<vector<int> > iivec;
	int row = strip_size;
	int col = NCFS_DATA->data_disk_num+1;
	for(int i = 0; i < col; i++){
		vector<int> ivec;
		for(int j = 0; j < strip_size; j++){
			if((mdr_I_encoding_matrixB[block_no*col+i]&(1<<(strip_size-j-1))) != 0){
				ivec.push_back(j);
			}
		}
		iivec.push_back(ivec);
	}
	return iivec;
}


vector<int> CodingLayer::mdr_I_repair_dpDisk_stripeIndexs_internal(int diskID, int val_k){
	vector<int> ivec;
	if(val_k == 1){
		if(diskID == 1){
			ivec.push_back(1);
			return ivec;
		}
		else if(diskID == 2){
			ivec.push_back(2);
			return ivec;
		}else{
			cout<<"diskID = "<<diskID<<endl;
			cout<<"val_k = "<<val_k<<endl;
			printf("error: fail diskID is large than k in mdr_I_repair_dpDisk_stripeIndexs()\n");
			exit(1);
		}
	}

	int val_strip_size = (int)pow(2, val_k);
	int r = val_strip_size / 2;
	if(diskID >= 1 && diskID <= val_k-1){ 
		vector<int> ivec_old = mdr_I_repair_dpDisk_stripeIndexs_internal(diskID, val_k-1);
		set<int> iset;
		iset.insert(ivec_old.begin(), ivec_old.end());

		for(int i = 1; i <= val_strip_size; i++){
			if((iset.find(i) != iset.end()) || (iset.find(i-r) != iset.end())){
				ivec.push_back(i);
			}
		}
		return ivec;
	}else if(diskID == val_k){
		for(int i = 1; i <= r; i++){
			ivec.push_back(i);
		}
		return ivec;
	}else if(diskID == val_k+1){
		for(int i = r+1; i <= val_strip_size; i++){
			ivec.push_back(i);
		}
		return ivec;
	}else{
		printf("error: fail diskID is large than k in mdr_I_repair_dpDisk_stripeIndexs()\n");
		exit(1);
	}
}

vector<int> CodingLayer::mdr_I_repair_dpDisk_stripeIndexs(int diskID, int val_k){
	vector<int> ivec = mdr_I_repair_dpDisk_stripeIndexs_internal(diskID+1, val_k);
	int ivec_size = ivec.size();

	vector<int> ivec2;
	for(int i = 0; i < ivec_size; i++){
		ivec2.push_back(ivec[i]-1);
	}
	return ivec2;
}

bool CodingLayer::mdr_I_repair_if_blk_in_buf(int disk_id, int stripe_blk_offset, bool** isInbuf,
									 vector<int>& stripeIndexs){
	int vec_size = stripeIndexs.size();
	for(int i = 0; i < vec_size; i++){
		if((disk_id == stripeIndexs[i]) && (isInbuf[i][stripe_blk_offset] == true)){
			return true;
		}
	}
	return false;
}


map<int, vector<vector<int> > > CodingLayer::mdr_I_repair_dpDisk_nonstripeIndexs_blocks_no(int fail_disk_id, 
														  vector<int>& stripeIndexs){

	map<int, vector<vector<int> > > ivmap;
	int row = strip_size;
	int col = NCFS_DATA->data_disk_num+1;

	set<int> iset;
	iset.insert(stripeIndexs.begin(), stripeIndexs.end());

	int stripeIndexs_size = stripeIndexs.size();
	for(int i = 0; i < stripeIndexs_size; i++){
		vector<vector<int> > iivec;
		int fail_disk_block_no = -1;
		for(int j = 0; j < col; j++){
			int val = mdr_I_encoding_matrixB[stripeIndexs[i]*col+j];
			//cout<<"val = "<<val<<endl;
			//bool flag = true;
			vector<int> ivec; 
			for(int t = 0; t < strip_size; t++){
				if((val&(1<<(strip_size-t-1))) != 0){
					if(iset.find(t) != iset.end()){
						//flag = false;
						ivec.push_back(t);
					}else if(iset.find(t) == iset.end() ){
						fail_disk_block_no = t;
					}
				}
			}
			// if(flag)
			// 	ivec.push_back(-1);
			iivec.push_back(ivec);
		}
		//cout<<endl;
		if(fail_disk_block_no == -1){
			printf("error in mdr_I_repair_dpDisk_nonstripeIndexs_blocks_no()\n");
			exit(1);
		}

		// push Q block
		vector<int> ivec;
		ivec.push_back(stripeIndexs[i]);
		iivec.push_back(ivec);

		ivmap[fail_disk_block_no] = iivec;
	}
	return ivmap;	
}

//Add by Dongsheng Wei on Jan. 17, 2014 end.



/*************************************************************************
 * Encoding functions
 *************************************************************************/

/*
 * default_encoding: select first available block
 *
 * @param buf: buffer of data to be written to disk
 * @param size: size of the data buffer
 *
 * @return: assigned data block: disk id and block number.
 */
struct data_block_info CodingLayer::encoding_default(const char *buf, int size)
{

	int retstat, disk_id, block_no, disk_total_num, block_size;
	int size_request, block_request, free_size;
	struct data_block_info block_written;
	int i;

	disk_total_num = NCFS_DATA->disk_total_num;
	block_size = NCFS_DATA->chunk_size;

	size_request = fileSystemLayer->round_to_block_size(size);
	block_request = size_request / block_size;

	//implement disk write algorithm here.
	//now use free block in bigger free space.

	block_no = 0;
	disk_id = -1;
	free_size = 0;
	for (i = 0; i < disk_total_num; i++) {
		if ((block_request <= (NCFS_DATA->free_size[i]))
		    && (free_size < (NCFS_DATA->free_size[i]))) {
			disk_id = i;
			block_no = NCFS_DATA->free_offset[i];
			free_size = NCFS_DATA->free_size[i];
		}
	}

	//get block from space_list if no free block available
	if (disk_id == -1) {
		if (NCFS_DATA->space_list_head != NULL) {
			disk_id = NCFS_DATA->space_list_head->disk_id;
			block_no = NCFS_DATA->space_list_head->disk_block_no;
			fileSystemLayer->space_list_remove(disk_id, block_no);
		}
	}

	if (disk_id == -1) {	//if no free block and no space finally
		printf("***get_data_block_no: ERROR disk_id = -1\n");
	} else {
		//Cache Start
		retstat =
		    cacheLayer->DiskWrite(disk_id, buf, size,
					  block_no * block_size);
		//Cache End

		NCFS_DATA->free_offset[disk_id] = block_no + block_request;
		NCFS_DATA->free_size[disk_id]
		    = NCFS_DATA->free_size[disk_id] - block_request;

	}

	block_written.disk_id = disk_id;
	block_written.block_no = block_no;

	return block_written;

}

/*
 * jbod_encoding: JBOD: non-stripped block allocation  (type=100)
 *
 * @param buf: buffer of data to be written to disk
 * @param size: size of the data buffer
 *
 * @return: assigned data block: disk id and block number.
 */
struct data_block_info CodingLayer::encoding_jbod(const char *buf, int size)
{
	int retstat, disk_id, block_no, disk_total_num, block_size;
	int size_request, block_request;
	struct data_block_info block_written;
	int i;

	disk_total_num = NCFS_DATA->disk_total_num;
	block_size = NCFS_DATA->chunk_size;

	size_request = fileSystemLayer->round_to_block_size(size);
	block_request = size_request / block_size;

	//implement disk write algorithm here.
	//here use jbod: first available block.

	block_no = 0;
	disk_id = -1;
	i = 0;
	while ((i < disk_total_num) && (disk_id == -1)) {
		if (block_request <= (NCFS_DATA->free_size[i])) {
			disk_id = i;
			block_no = NCFS_DATA->free_offset[i];
		}
		i++;
	}

	//get block from space_list if no free block available
	if (disk_id == -1) {
		if (NCFS_DATA->space_list_head != NULL) {
			disk_id = NCFS_DATA->space_list_head->disk_id;
			block_no = NCFS_DATA->space_list_head->disk_block_no;
			fileSystemLayer->space_list_remove(disk_id, block_no);
		}
	}

	if (disk_id == -1) {
		printf("***get_data_block_no: ERROR disk_id = -1\n");
	} else {
		//Cache Start

		retstat =
		    cacheLayer->DiskWrite(disk_id, buf, size,
					  block_no * block_size);

		NCFS_DATA->free_offset[disk_id] = block_no + block_request;
		NCFS_DATA->free_size[disk_id]
		    = NCFS_DATA->free_size[disk_id] - block_request;

		//Cache End
	}

	block_written.disk_id = disk_id;
	block_written.block_no = block_no;

	return block_written;

}

/*
 * raid0_encoding: RAID 0: stripped block allocation (type=0)
 *
 * @param buf: buffer of data to be written to disk
 * @param size: size of the data buffer
 *
 * @return: assigned data block: disk id and block number.
 */
struct data_block_info CodingLayer::encoding_raid0(const char *buf, int size)
{
	int retstat, disk_id, block_no, disk_total_num, block_size;
	int size_request, block_request, free_offset;
	struct data_block_info block_written;
	int i;

	disk_total_num = NCFS_DATA->disk_total_num;
	block_size = NCFS_DATA->chunk_size;

	size_request = fileSystemLayer->round_to_block_size(size);
	block_request = size_request / block_size;

	//implement disk write algorithm here.
	//here use raid0: stripped block allocation.

	block_no = 0;
	disk_id = -1;
	free_offset = NCFS_DATA->free_offset[0];
	for (i = disk_total_num - 1; i >= 0; i--) {
		if ((block_request <= (NCFS_DATA->free_size[i]))
		    && (free_offset >= (NCFS_DATA->free_offset[i]))) {
			disk_id = i;
			block_no = NCFS_DATA->free_offset[i];
			free_offset = block_no;
		}
	}

	//get block from space_list if no free block available
	if (disk_id == -1) {
		if (NCFS_DATA->space_list_head != NULL) {
			disk_id = NCFS_DATA->space_list_head->disk_id;
			block_no = NCFS_DATA->space_list_head->disk_block_no;
			fileSystemLayer->space_list_remove(disk_id, block_no);
		}
	}

	if (disk_id == -1) {
		printf("***get_data_block_no: ERROR disk_id = -1\n");
	} else {
		//Cache Start
		retstat =
		    cacheLayer->DiskWrite(disk_id, buf, size,
					  block_no * block_size);

		NCFS_DATA->free_offset[disk_id] = block_no + block_request;
		NCFS_DATA->free_size[disk_id]
		    = NCFS_DATA->free_size[disk_id] - block_request;
		//Cache End
	}

	block_written.disk_id = disk_id;
	block_written.block_no = block_no;

	return block_written;

}

/*
 * raid1_encoding: RAID 1: fault tolerance by mirror (type=1)
 *
 * @param buf: buffer of data to be written to disk
 * @param size: size of the data buffer
 *
 * @return: assigned data block: disk id and block number.
 */
struct data_block_info CodingLayer::encoding_raid1(const char *buf, int size)
{
	int retstat, disk_id, block_no, disk_total_num, block_size;
	int size_request, block_request;
	struct data_block_info block_written;
	int i;
	int mirror_disk_id;

	disk_total_num = NCFS_DATA->disk_total_num;
	block_size = NCFS_DATA->chunk_size;

	size_request = fileSystemLayer->round_to_block_size(size);
	block_request = size_request / block_size;

	//implement disk write algorithm here.
	//here use raid1: mirroring. (with jbod)

	block_no = 0;
	disk_id = -1;
	i = 0;
	//first n/2 disks are data disks
	while ((i < disk_total_num / 2) && (disk_id == -1)) {
		if (block_request <= (NCFS_DATA->free_size[i])) {
			disk_id = i;
			block_no = NCFS_DATA->free_offset[i];
		}
		i++;
	}

	//get block from space_list if no free block available
	if (disk_id == -1) {
		if (NCFS_DATA->space_list_head != NULL) {
			disk_id = NCFS_DATA->space_list_head->disk_id;
			block_no = NCFS_DATA->space_list_head->disk_block_no;
			fileSystemLayer->space_list_remove(disk_id, block_no);
		}
	}

	if (disk_id == -1) {
		printf("***get_data_block_no: ERROR disk_id = -1\n");
	} else {
		//Cache Start
		retstat =
		    cacheLayer->DiskWrite(disk_id, buf, size,
					  block_no * block_size);
		//Cache End

		NCFS_DATA->free_offset[disk_id] = block_no + block_request;
		NCFS_DATA->free_size[disk_id]
		    = NCFS_DATA->free_size[disk_id] - block_request;

		//write data to mirror disk
		mirror_disk_id = disk_total_num - disk_id - 1;
		//Cache Start
		retstat =
		    cacheLayer->DiskWrite(mirror_disk_id, buf, size,
					  block_no * block_size);
		//Cache End
	}

	block_written.disk_id = disk_id;
	block_written.block_no = block_no;

	return block_written;
}

/*
 * raid4_encoding: RAID 4: fault tolerance by non-stripped parity (type=4)
 *
 * @param buf: buffer of data to be written to disk
 * @param size: size of the data buffer
 *
 * @return: assigned data block: disk id and block number.
 */
struct data_block_info CodingLayer::encoding_raid4(const char *buf, int size)
{
	int retstat, disk_id, block_no, disk_total_num, block_size;
	int size_request, block_request, free_offset;
	struct data_block_info block_written;
	int i, j;
	int parity_disk_id;
	char *buf2, *buf_read;

	disk_total_num = NCFS_DATA->disk_total_num;
	block_size = NCFS_DATA->chunk_size;

	size_request = fileSystemLayer->round_to_block_size(size);
	block_request = size_request / block_size;

	//implement disk write algorithm here.
	//here use raid4: stripped block allocation plus dedicated parity disk.
	//use the last disk as parity disk.
	//approach: calculate xor of the block on all data disk 
	block_no = 0;
	disk_id = -1;
	free_offset = NCFS_DATA->free_offset[0];
	for (i = disk_total_num - 2; i >= 0; i--) {
		if ((block_request <= (NCFS_DATA->free_size[i]))
		    && (free_offset >= (NCFS_DATA->free_offset[i]))) {
			disk_id = i;
			block_no = NCFS_DATA->free_offset[i];
			free_offset = block_no;
		}
	}

	//get block from space_list if no free block available
	if (disk_id == -1) {
		if (NCFS_DATA->space_list_head != NULL) {
			disk_id = NCFS_DATA->space_list_head->disk_id;
			block_no = NCFS_DATA->space_list_head->disk_block_no;
			fileSystemLayer->space_list_remove(disk_id, block_no);
		}
	}

	if (disk_id == -1) {
		printf("***get_data_block_no: ERROR disk_id = -1\n");
	} else {
		buf_read = (char *)malloc(sizeof(char) * size_request);
		buf2 = (char *)malloc(sizeof(char) * size_request);
		//Cache Start
		retstat =
		    cacheLayer->DiskRead(disk_id, buf2, size_request,
					 block_no * block_size);
		//Cache End
		for (j = 0; j < size_request; j++) {
			buf2[j] = buf2[j] ^ buf[j];
		}
		retstat =
		    cacheLayer->DiskWrite(disk_id, buf, size,
					  block_no * block_size);

		NCFS_DATA->free_offset[disk_id] = block_no + block_request;
		NCFS_DATA->free_size[disk_id]
		    = NCFS_DATA->free_size[disk_id] - block_request;

		parity_disk_id = disk_total_num - 1;
		retstat =
		    cacheLayer->DiskRead(parity_disk_id, buf_read, size_request,
					 block_no * block_size);
		for (j = 0; j < size_request; j++) {
			buf2[j] = buf2[j] ^ buf_read[j];
		}

		retstat =
		    cacheLayer->DiskWrite(parity_disk_id, buf2, size,
					  block_no * block_size);

		free(buf_read);
		free(buf2);
	}

	block_written.disk_id = disk_id;
	block_written.block_no = block_no;

	return block_written;
}

/*
 * raid5_encoding: RAID 5: fault tolerance by stripped parity (type=5)
 * 
 * @param buf: buffer of data to be written to disk
 * @param size: size of the data buffer
 *
 * @return: assigned data block: disk id and block number.
 */
struct data_block_info CodingLayer::encoding_raid5(const char *buf, int size)
{
	int retstat, disk_id, block_no, disk_total_num, block_size;
	int size_request, block_request, free_offset;
	struct data_block_info block_written;
	int i, j;
	int parity_disk_id;
	char *buf2, *buf_read;

	struct timeval t1, t2, t3, t4, t5, t6;
	double duration;

	disk_total_num = NCFS_DATA->disk_total_num;
	block_size = NCFS_DATA->chunk_size;

	size_request = fileSystemLayer->round_to_block_size(size);
	block_request = size_request / block_size;

	//implement disk write algorithm here.
	//here use raid5: stripped block allocation plus distributed parity.
	//approach: calculate xor of the block on all data disk
	for (i = 0; i < disk_total_num; i++) {
		if (i ==
		    (disk_total_num - 1 -
		     (NCFS_DATA->free_offset[i] % disk_total_num))) {
			(NCFS_DATA->free_offset[i])++;
			(NCFS_DATA->free_size[i])--;
		}
	}

	block_no = 0;
	disk_id = -1;
	free_offset = NCFS_DATA->free_offset[0];
	for (i = disk_total_num - 1; i >= 0; i--) {
		if ((block_request <= (NCFS_DATA->free_size[i]))
		    && (free_offset >= (NCFS_DATA->free_offset[i]))) {
			disk_id = i;
			block_no = NCFS_DATA->free_offset[i];
			free_offset = block_no;
		}
	}

	//get block from space_list if no free block available
	if (disk_id == -1) {
		if (NCFS_DATA->space_list_head != NULL) {
			disk_id = NCFS_DATA->space_list_head->disk_id;
			block_no = NCFS_DATA->space_list_head->disk_block_no;
			fileSystemLayer->space_list_remove(disk_id, block_no);
		}
	}

	if (disk_id == -1) {
		printf("***get_data_block_no: ERROR disk_id = -1\n");
	} else {
		buf_read = (char *)malloc(sizeof(char) * size_request);
		buf2 = (char *)malloc(sizeof(char) * size_request);

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t1, NULL);
		}
		//Cache Start
		retstat =
		    cacheLayer->DiskRead(disk_id, buf2, size_request,
					 block_no * block_size);
		//Cache End

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t2, NULL);
		}

		for (j = 0; j < size_request; j++) {
			buf2[j] = buf2[j] ^ buf[j];
		}

		NCFS_DATA->free_offset[disk_id] = block_no + block_request;
		NCFS_DATA->free_size[disk_id]
		    = NCFS_DATA->free_size[disk_id] - block_request;

		parity_disk_id =
		    disk_total_num - 1 - (block_no % disk_total_num);

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t3, NULL);
		}
		//Cache Start
		retstat =
		    cacheLayer->DiskRead(parity_disk_id, buf_read, size_request,
					 block_no * block_size);
		//Cache End

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t4, NULL);
		}

		for (j = 0; j < size_request; j++) {
			buf2[j] = buf2[j] ^ buf_read[j];
		}

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t5, NULL);
		}
		//Cache Start
		//write data
		retstat =
		    cacheLayer->DiskWrite(disk_id, buf, size,
					  block_no * block_size);
		//Cache End

		//Cache Start
		//write parity
		retstat =
		    cacheLayer->DiskWrite(parity_disk_id, buf2, size,
					  block_no * block_size);
		//Cache End

		free(buf_read);
		free(buf2);

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t6, NULL);

			duration = (t3.tv_sec - t2.tv_sec) +
			    (t3.tv_usec - t2.tv_usec) / 1000000.0;
			NCFS_DATA->encoding_time += duration;

			duration = (t5.tv_sec - t4.tv_sec) +
			    (t5.tv_usec - t4.tv_usec) / 1000000.0;
			NCFS_DATA->encoding_time += duration;
		}
	}

	block_written.disk_id = disk_id;
	block_written.block_no = block_no;

	return block_written;
}

/*
 * raid6_encoding: RAID 6: fault tolerance by stripped parity (type=6)
 *
 * @param buf: buffer of data to be written to disk
 * @param size: size of the data buffer
 *
 * @return: assigned data block: disk id and block number.
 */
struct data_block_info CodingLayer::encoding_raid6(const char *buf, int size)
{
	int retstat, disk_id, block_no, disk_total_num, block_size;
	int size_request, block_request, free_offset;
	struct data_block_info block_written;
	int i, j;
	int parity_disk_id, code_disk_id;
	char *buf2, *buf3, *buf_read;
	char temp_char;
	int data_disk_coeff;

	struct timeval t1, t2, t3;
	double duration;

	disk_total_num = NCFS_DATA->disk_total_num;
	block_size = NCFS_DATA->chunk_size;

	size_request = fileSystemLayer->round_to_block_size(size);
	block_request = size_request / block_size;

	//implement disk write algorithm here.
	//here use raid6: stripped block allocation plus distributed parity and distributed RS code.

	for (i = 0; i < disk_total_num; i++) {
		//mark blocks of code_disk and parity_disk
		if ((i ==
		     (disk_total_num - 1 -
		      (NCFS_DATA->free_offset[i] % disk_total_num)))
		    || (i ==
			(disk_total_num - 1 -
			 ((NCFS_DATA->free_offset[i] + 1) % disk_total_num)))) {
			(NCFS_DATA->free_offset[i])++;
			(NCFS_DATA->free_size[i])--;
		}
	}

	block_no = 0;
	disk_id = -1;
	free_offset = NCFS_DATA->free_offset[0];
	for (i = disk_total_num - 1; i >= 0; i--) {
		if ((block_request <= (NCFS_DATA->free_size[i]))
		    && (free_offset >= (NCFS_DATA->free_offset[i]))) {
			disk_id = i;
			block_no = NCFS_DATA->free_offset[i];
			free_offset = block_no;
		}
	}


	//get block from space_list if no free block available
	if (disk_id == -1) {
		if (NCFS_DATA->space_list_head != NULL) {
			disk_id = NCFS_DATA->space_list_head->disk_id;
			block_no = NCFS_DATA->space_list_head->disk_block_no;
			fileSystemLayer->space_list_remove(disk_id, block_no);
		}
	}

	if (disk_id == -1) {
		printf("***get_data_block_no: ERROR disk_id = -1\n");
	} else {
		buf_read = (char *)malloc(sizeof(char) * size_request);
		buf2 = (char *)malloc(sizeof(char) * size_request);
		memset(buf2, 0, size_request);
		buf3 = (char *)malloc(sizeof(char) * size_request);
		memset(buf3, 0, size_request);

		NCFS_DATA->free_offset[disk_id] = block_no + block_request;
		NCFS_DATA->free_size[disk_id]
		    = NCFS_DATA->free_size[disk_id] - block_request;

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t1, NULL);
		}
		// Cache Start
		retstat =
		    cacheLayer->DiskWrite(disk_id, buf, size,
					  block_no * block_size);
		// Cache End

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t2, NULL);
		}

		code_disk_id = disk_total_num - 1 - (block_no % disk_total_num);
		parity_disk_id =
		    disk_total_num - 1 - ((block_no + 1) % disk_total_num);

		for (i = 0; i < disk_total_num; i++) {
			if ((i != parity_disk_id) && (i != code_disk_id)) {

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t1, NULL);
				}
				//Cache Start
				retstat = cacheLayer->DiskRead(i, buf_read,
							       size_request,
							       block_no *
							       block_size);
				//Cache End

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t2, NULL);
				}

				for (j = 0; j < size_request; j++) {
					//Calculate parity block P
					buf2[j] = buf2[j] ^ buf_read[j];

					//calculate the coefficient of the data block
					data_disk_coeff = i;

					if (i > code_disk_id) {
						(data_disk_coeff)--;
					}
					if (i > parity_disk_id) {
						(data_disk_coeff)--;
					}
					data_disk_coeff =
					    disk_total_num - 3 -
					    data_disk_coeff;
					data_disk_coeff =
					    gf_get_coefficient(data_disk_coeff,
							       field_power);

					//calculate code block Q
					temp_char = buf3[j];
					buf3[j] = temp_char ^
					    (char)gf_mul((unsigned char)
							 buf_read[j],
							 data_disk_coeff,
							 field_power);
				}

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t3, NULL);

					duration = (t3.tv_sec - t2.tv_sec) +
					    (t3.tv_usec -
					     t2.tv_usec) / 1000000.0;
					NCFS_DATA->encoding_time += duration;
				}
			}
		}

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t1, NULL);
		}
		// Cache Start
		retstat =
		    cacheLayer->DiskWrite(parity_disk_id, buf2, size,
					  block_no * block_size);
		retstat =
		    cacheLayer->DiskWrite(code_disk_id, buf3, size,
					  block_no * block_size);
		// Cache End

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t2, NULL);
		}

		free(buf_read);
		free(buf2);
		free(buf3);
	}

	block_written.disk_id = disk_id;
	block_written.block_no = block_no;

	return block_written;
}


//Add by Dongsheng Wei on Jan. 16, 2014 begin.

struct data_block_info CodingLayer::encoding_mdr_I(const char *buf, int size)
{
	int retstat, disk_id, block_no, disk_total_num, block_size;
	int size_request, block_request, free_offset;
	struct data_block_info block_written;
	int i, j;
	int parity_disk_id, code_disk_id;
	char *buf2, *buf3, *buf_read;
	//dongsheng wei
	char *buf_parity_disk, *buf_code_disk;
	int data_disk_num;
	vector<int> q_blocks_no;
//	int strip_size;
	//dongsheng wei
	char temp_char;
	int data_disk_coeff;

	struct timeval t1, t2, t3;
	//dongsheng wei
	struct timeval t4, t5, t6;
	//dongsheng wei
	double duration;

	disk_total_num = NCFS_DATA->disk_total_num;
	block_size = NCFS_DATA->chunk_size;

	size_request = fileSystemLayer->round_to_block_size(size);
	block_request = size_request / block_size;

	data_disk_num = disk_total_num - 2;




	//implement disk write algorithm here.
	//here use raid6.

	//dognsheng wei
	// for (i = 0; i < disk_total_num; i++) {
	// 	//mark blocks of code_disk and parity_disk
	// 	if ((i ==
	// 	     (disk_total_num - 1 -
	// 	      (NCFS_DATA->free_offset[i] % disk_total_num)))
	// 	    || (i ==
	// 		(disk_total_num - 1 -
	// 		 ((NCFS_DATA->free_offset[i] + 1) % disk_total_num)))) {
	// 		(NCFS_DATA->free_offset[i])++;
	// 		(NCFS_DATA->free_size[i])--;
	// 	}
	// }

	block_no = 0;
	disk_id = -1;
	free_offset = NCFS_DATA->free_offset[0];
	//dongsheng wei
	for (i = disk_total_num - 3; i >= 0; i--) {
		if ((block_request <= (NCFS_DATA->free_size[i]))
		    && (free_offset >= (NCFS_DATA->free_offset[i]))) {
			disk_id = i;
			block_no = NCFS_DATA->free_offset[i];
			free_offset = block_no;
		}
	}

	//dongsheng wei
	if(disk_id == 0){
		(NCFS_DATA->free_offset[disk_total_num-2])++;
		(NCFS_DATA->free_size[disk_total_num-2])--;
		if(block_no % strip_size == 0){
			(NCFS_DATA->free_offset[disk_total_num-1]) += strip_size;
			(NCFS_DATA->free_size[disk_total_num-1]) -= strip_size;
		}
	}

	//get block from space_list if no free block available
	if (disk_id == -1) {
		if (NCFS_DATA->space_list_head != NULL) {
			disk_id = NCFS_DATA->space_list_head->disk_id;
			block_no = NCFS_DATA->space_list_head->disk_block_no;
			fileSystemLayer->space_list_remove(disk_id, block_no);
		}
	}

	if (disk_id == -1) {
		printf("***get_data_block_no: ERROR disk_id = -1\n");
	} else {
		NCFS_DATA->free_offset[disk_id] = block_no + block_request;
		NCFS_DATA->free_size[disk_id]
		    = NCFS_DATA->free_size[disk_id] - block_request;

		int p_disk_id = disk_total_num - 2;
		int q_disk_id = disk_total_num - 1;
		
		// //dongsheng wei
		char* buf_p_disk = (char *)malloc(sizeof(char) * size_request);
		char* buf_q_disk = (char *)malloc(sizeof(char) * size_request);

		
		//read P block
		retstat = cacheLayer->DiskRead(p_disk_id, buf_p_disk, 
			size_request, block_no * block_size);		
		
		//calculate new P block
		for (j = 0; j < size_request; j++) {
			buf_p_disk[j] = buf_p_disk[j] ^ buf[j];
		}

		//write new P block
		retstat = cacheLayer->DiskWrite(p_disk_id, buf_p_disk, size, 
			block_no * block_size);		

		//read Q block
		int strip_num = block_no / strip_size;
		int strip_offset = block_no % strip_size;

		q_blocks_no = mdr_I_find_q_blocks_id(disk_id, strip_offset);

		// cout<<"q_blocks_no\n";
		// print_ivec(q_blocks_no);

		int q_blk_num = q_blocks_no.size();
		for(i = 0; i < q_blk_num; i++){
			int q_blk_no = q_blocks_no[i] + strip_num * strip_size;

			//read Q blk
			memset(buf_q_disk, 0, size_request);

			retstat = cacheLayer->DiskRead(q_disk_id, buf_q_disk, 
				size_request, q_blk_no * block_size);	

			//calculate new Q blk
			for (j = 0; j < size_request; j++) {
				buf_q_disk[j] = buf_q_disk[j] ^ buf[j];
			}

			//write new Q blk
			retstat = cacheLayer->DiskWrite(q_disk_id, buf_q_disk, size, 
				q_blk_no * block_size);			
		}

		//write buf
		retstat = cacheLayer->DiskWrite(disk_id, buf, size, 
			block_no * block_size);			

		free(buf_p_disk);
		free(buf_q_disk);
	}

	block_written.disk_id = disk_id;
	block_written.block_no = block_no;

	return block_written;
}

struct data_block_info CodingLayer::encoding_raid6_noRotate(const char *buf, int size)
{
	int retstat, disk_id, block_no, disk_total_num, block_size;
	int size_request, block_request, free_offset;
	struct data_block_info block_written;
	int i, j;
	int parity_disk_id, code_disk_id;
	char *buf2, *buf3, *buf_read;
	char temp_char;
	int data_disk_coeff;

	struct timeval t1, t2, t3;
	double duration;

	disk_total_num = NCFS_DATA->disk_total_num;
	block_size = NCFS_DATA->chunk_size;

	size_request = fileSystemLayer->round_to_block_size(size);
	block_request = size_request / block_size;

	//implement disk write algorithm here.
	//here use raid6.

	//dognsheng wei
	// for (i = 0; i < disk_total_num; i++) {
	// 	//mark blocks of code_disk and parity_disk
	// 	if ((i ==
	// 	     (disk_total_num - 1 -
	// 	      (NCFS_DATA->free_offset[i] % disk_total_num)))
	// 	    || (i ==
	// 		(disk_total_num - 1 -
	// 		 ((NCFS_DATA->free_offset[i] + 1) % disk_total_num)))) {
	// 		(NCFS_DATA->free_offset[i])++;
	// 		(NCFS_DATA->free_size[i])--;
	// 	}
	// }

	block_no = 0;
	disk_id = -1;
	free_offset = NCFS_DATA->free_offset[0];
	//dongsheng wei
	for (i = disk_total_num - 3; i >= 0; i--) {
		if ((block_request <= (NCFS_DATA->free_size[i]))
		    && (free_offset >= (NCFS_DATA->free_offset[i]))) {
			disk_id = i;
			block_no = NCFS_DATA->free_offset[i];
			free_offset = block_no;
		}
	}

	//dongsheng wei
	if(disk_id == 0){
		(NCFS_DATA->free_offset[disk_total_num-2])++;
		(NCFS_DATA->free_size[disk_total_num-2])--;
		(NCFS_DATA->free_offset[disk_total_num-1])++;
		(NCFS_DATA->free_size[disk_total_num-1])--;
	}

	//get block from space_list if no free block available
	if (disk_id == -1) {
		if (NCFS_DATA->space_list_head != NULL) {
			disk_id = NCFS_DATA->space_list_head->disk_id;
			block_no = NCFS_DATA->space_list_head->disk_block_no;
			fileSystemLayer->space_list_remove(disk_id, block_no);
		}
	}

	if (disk_id == -1) {
		printf("***get_data_block_no: ERROR disk_id = -1\n");
	} else {
		buf_read = (char *)malloc(sizeof(char) * size_request);
		buf2 = (char *)malloc(sizeof(char) * size_request);
		memset(buf2, 0, size_request);
		buf3 = (char *)malloc(sizeof(char) * size_request);
		memset(buf3, 0, size_request);

		NCFS_DATA->free_offset[disk_id] = block_no + block_request;
		NCFS_DATA->free_size[disk_id]
		    = NCFS_DATA->free_size[disk_id] - block_request;

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t1, NULL);
		}
		// Cache Start
		retstat =
		    cacheLayer->DiskWrite(disk_id, buf, size,
					  block_no * block_size);
		// Cache End

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t2, NULL);
		}

		//dongsheng wei
		// code_disk_id = disk_total_num - 1 - (block_no % disk_total_num);
		// parity_disk_id =
		//     disk_total_num - 1 - ((block_no + 1) % disk_total_num);

		code_disk_id = disk_total_num - 1;
		parity_disk_id = disk_total_num-2;

		for (i = 0; i < disk_total_num; i++) {
			if ((i != parity_disk_id) && (i != code_disk_id)) {

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t1, NULL);
				}
				//Cache Start
				retstat = cacheLayer->DiskRead(i, buf_read,
							       size_request,
							       block_no *
							       block_size);
				//Cache End

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t2, NULL);
				}

				for (j = 0; j < size_request; j++) {
					//Calculate parity block P
					buf2[j] = buf2[j] ^ buf_read[j];

					//calculate the coefficient of the data block
					data_disk_coeff = i;

					if (i > code_disk_id) {
						(data_disk_coeff)--;
					}
					if (i > parity_disk_id) {
						(data_disk_coeff)--;
					}
					data_disk_coeff =
					    disk_total_num - 3 -
					    data_disk_coeff;
					data_disk_coeff =
					    gf_get_coefficient(data_disk_coeff,
							       field_power);

					//calculate code block Q
					temp_char = buf3[j];
					buf3[j] = temp_char ^
					    (char)gf_mul((unsigned char)
							 buf_read[j],
							 data_disk_coeff,
							 field_power);
				}

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t3, NULL);

					duration = (t3.tv_sec - t2.tv_sec) +
					    (t3.tv_usec -
					     t2.tv_usec) / 1000000.0;
					NCFS_DATA->encoding_time += duration;
				}
			}
		}

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t1, NULL);
		}
		// Cache Start
		retstat =
		    cacheLayer->DiskWrite(parity_disk_id, buf2, size,
					  block_no * block_size);
		retstat =
		    cacheLayer->DiskWrite(code_disk_id, buf3, size,
					  block_no * block_size);
		// Cache End

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t2, NULL);
		}

		free(buf_read);
		free(buf2);
		free(buf3);
	}

	block_written.disk_id = disk_id;
	block_written.block_no = block_no;

	return block_written;
}

struct data_block_info CodingLayer::encoding_raid5_noRotate(const char *buf, int size)
{
	//RAID 5 with no rotate

	int retstat, disk_id, block_no, disk_total_num, block_size;
	int size_request, block_request, free_offset;
	struct data_block_info block_written;
	int i, j;
	int parity_disk_id;
	char *buf2, *buf_read;

	struct timeval t1, t2, t3, t4, t5, t6;
	double duration;

	disk_total_num = NCFS_DATA->disk_total_num;
	block_size = NCFS_DATA->chunk_size;

	size_request = fileSystemLayer->round_to_block_size(size);
	block_request = size_request / block_size;

	//implement disk write algorithm here.
	//here use raid5: stripped block allocation plus distributed parity.
	//approach: calculate xor of the block on all data disk

	// //dongsheng wei
	// for (i = 0; i < disk_total_num; i++) {
	// 	if (i ==
	// 	    (disk_total_num - 1 -
	// 	     (NCFS_DATA->free_offset[i] % disk_total_num))) {
	// 		(NCFS_DATA->free_offset[i])++;
	// 		(NCFS_DATA->free_size[i])--;
	// 	}
	// }

	block_no = 0;
	disk_id = -1;
	free_offset = NCFS_DATA->free_offset[0];
	//dongsheng wei
	for (i = disk_total_num - 2; i >= 0; i--) {
		if ((block_request <= (NCFS_DATA->free_size[i]))
		    && (free_offset >= (NCFS_DATA->free_offset[i]))) {
			disk_id = i;
			block_no = NCFS_DATA->free_offset[i];
			free_offset = block_no;
		}
	}


	//dongsheng wei
	printf("\n\ndisk_id = %d\n\n", disk_id);
	if(disk_id == 0){
		(NCFS_DATA->free_offset[disk_total_num-1])++;
		(NCFS_DATA->free_size[disk_total_num-1])--;
	}

	//get block from space_list if no free block available
	if (disk_id == -1) {
		if (NCFS_DATA->space_list_head != NULL) {
			disk_id = NCFS_DATA->space_list_head->disk_id;
			block_no = NCFS_DATA->space_list_head->disk_block_no;
			fileSystemLayer->space_list_remove(disk_id, block_no);
		}
	}

	if (disk_id == -1) {
		printf("***get_data_block_no: ERROR disk_id = -1\n");
	} else {
		buf_read = (char *)malloc(sizeof(char) * size_request);
		buf2 = (char *)malloc(sizeof(char) * size_request);

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t1, NULL);
		}
		//Cache Start
		retstat =
		    cacheLayer->DiskRead(disk_id, buf2, size_request,
					 block_no * block_size);
		//Cache End

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t2, NULL);
		}

		for (j = 0; j < size_request; j++) {
			buf2[j] = buf2[j] ^ buf[j];
		}

		NCFS_DATA->free_offset[disk_id] = block_no + block_request;
		NCFS_DATA->free_size[disk_id]
		    = NCFS_DATA->free_size[disk_id] - block_request;

		// parity_disk_id = 
		//     disk_total_num - 1 - (block_no % disk_total_num);

		//dongsheng wei
		parity_disk_id = disk_total_num - 1;
		//dongsheng wei

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t3, NULL);
		}
		//Cache Start
		retstat =
		    cacheLayer->DiskRead(parity_disk_id, buf_read, size_request,
					 block_no * block_size);
		//Cache End

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t4, NULL);
		}

		for (j = 0; j < size_request; j++) {
			buf2[j] = buf2[j] ^ buf_read[j];
		}

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t5, NULL);
		}
		//Cache Start
		//write data
		retstat =
		    cacheLayer->DiskWrite(disk_id, buf, size,
					  block_no * block_size);
		//Cache End

		//Cache Start
		//write parity
		retstat =
		    cacheLayer->DiskWrite(parity_disk_id, buf2, size,
					  block_no * block_size);
		//Cache End

		free(buf_read);
		free(buf2);

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t6, NULL);

			duration = (t3.tv_sec - t2.tv_sec) +
			    (t3.tv_usec - t2.tv_usec) / 1000000.0;
			NCFS_DATA->encoding_time += duration;

			duration = (t5.tv_sec - t4.tv_sec) +
			    (t5.tv_usec - t4.tv_usec) / 1000000.0;
			NCFS_DATA->encoding_time += duration;
		}
	}

	block_written.disk_id = disk_id;
	block_written.block_no = block_no;

	return block_written;
}

//Add by Dongsheng Wei on Jan. 16, 2014 end.


/*
 * mbr_encoding: MBR: Minimum Bandwidth Regenerating code (type=1000)
 *
 * @param buf: buffer of data to be written to disk
 * @param size: size of the data buffer
 *
 * @return: assigned data block: disk id and block number.
 */
struct data_block_info CodingLayer::encoding_mbr(const char *buf, int size)
{

	int retstat, disk_id, block_no, disk_total_num, block_size;
	int size_request, block_request, free_offset;
	struct data_block_info block_written;
	int i, j;
	char *buf2, *buf_read, *buf_temp;

	int mbr_segment_size;
	int mbr_segment_id;
	int mbr_native_block_id;
	int mbr_code_block_id;
	int dup_disk_id, dup_block_no, code_disk_id, code_block_no;

	struct timeval t1, t2, t3;
	double duration;

	mbr_segment_size = NCFS_DATA->mbr_segment_size;

	disk_total_num = NCFS_DATA->disk_total_num;
	block_size = NCFS_DATA->chunk_size;

	size_request = fileSystemLayer->round_to_block_size(size);
	block_request = size_request / block_size;

	//implement disk write algorithm here.
	//here use mbr.
	//approach: write original data block, duplicated block and coded block
	for (i = 0; i < disk_total_num; i++) {
		mbr_native_block_id =
		    mbr_find_block_id(i, NCFS_DATA->free_offset[i],
				      mbr_segment_size);
		if ((mbr_native_block_id == -1)
		    || (mbr_native_block_id >= NCFS_DATA->mbr_m)) {
			(NCFS_DATA->free_offset[i])++;
			(NCFS_DATA->free_size[i])--;
		}
	}

	block_no = 0;
	disk_id = -1;
	free_offset = NCFS_DATA->free_offset[0];
	for (i = disk_total_num - 1; i >= 0; i--) {
		mbr_native_block_id =
		    mbr_find_block_id(i, NCFS_DATA->free_offset[i],
				      mbr_segment_size);
		if ((block_request <= (NCFS_DATA->free_size[i]))
		    && (free_offset >= (NCFS_DATA->free_offset[i]))
		    && (mbr_native_block_id != -1)
		    && (mbr_native_block_id < NCFS_DATA->mbr_m)) {
			disk_id = i;
			block_no = NCFS_DATA->free_offset[i];
			free_offset = block_no;
		}
	}

	//get block from space_list if no free block available
	if (disk_id == -1) {
		if (NCFS_DATA->space_list_head != NULL) {
			disk_id = NCFS_DATA->space_list_head->disk_id;
			block_no = NCFS_DATA->space_list_head->disk_block_no;
			fileSystemLayer->space_list_remove(disk_id, block_no);
		}
	}

	if (disk_id == -1) {
		printf("***get_data_block_no: ERROR disk_id = -1\n");
	} else {
		mbr_segment_id = (int)(block_no / mbr_segment_size);

		//write original data block
		buf_read = (char *)malloc(sizeof(char) * size_request);
		buf2 = (char *)malloc(sizeof(char) * size_request);
		buf_temp = (char *)malloc(sizeof(char) * size_request);

		//attention 1, read origin data block XOR new written block
		if (NCFS_DATA->mbr_c > 0) {

			if (NCFS_DATA->run_experiment == 1) {
				gettimeofday(&t1, NULL);
			}
			//Cache Start
			retstat =
			    cacheLayer->DiskRead(disk_id, buf2, size_request,
						 block_no * block_size);
			//Cache End

			if (NCFS_DATA->run_experiment == 1) {
				gettimeofday(&t2, NULL);
			}

			for (j = 0; j < size_request; j++) {
				buf2[j] = buf2[j] ^ buf[j];
			}

			if (NCFS_DATA->run_experiment == 1) {
				gettimeofday(&t3, NULL);

				duration = (t3.tv_sec - t2.tv_sec) +
				    (t3.tv_usec - t2.tv_usec) / 1000000.0;
				NCFS_DATA->encoding_time += duration;
			}
		}

		NCFS_DATA->free_offset[disk_id] = block_no + block_request;
		NCFS_DATA->free_size[disk_id]
		    = NCFS_DATA->free_size[disk_id] - block_request;

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t1, NULL);
		}
		//write duplicated block
		mbr_native_block_id =
		    mbr_find_block_id(disk_id, block_no, mbr_segment_size);
		dup_disk_id =
		    mbr_get_dup_disk_id(mbr_native_block_id, mbr_segment_size);
		dup_block_no =
		    mbr_get_dup_block_no(disk_id, mbr_native_block_id,
					 mbr_segment_size);

		dup_block_no = dup_block_no + mbr_segment_id * mbr_segment_size;

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t2, NULL);
		}
		//Cache Start
		//write data
		retstat =
		    cacheLayer->DiskWrite(disk_id, buf, size,
					  block_no * block_size);
		//write duplicate data
		retstat =
		    cacheLayer->DiskWrite(dup_disk_id, buf, size,
					  dup_block_no * block_size);
		//Cache End

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t3, NULL);

			duration = (t2.tv_sec - t1.tv_sec) +
			    (t2.tv_usec - t1.tv_usec) / 1000000.0;
			NCFS_DATA->encoding_time += duration;
		}

		if (NCFS_DATA->no_gui == 0) {
			diskusageLayer->Update(dup_disk_id, dup_block_no, 1);
		}
		//attention 2, make the new encoded block and write the encoded block
		for (i = 0; i < NCFS_DATA->mbr_c; i++) {

			memset(buf_temp, 0, size_request);
			memset(buf_read, 0, size_request);
			//generate coded block data

			//use the coded block one by one.
			mbr_code_block_id = (NCFS_DATA->mbr_m) + i;
			code_disk_id =
			    mbr_get_disk_id(mbr_code_block_id,
					    mbr_segment_size);

			code_block_no =
			    mbr_get_block_no(code_disk_id, mbr_code_block_id,
					     mbr_segment_size);

			code_block_no =
			    code_block_no + mbr_segment_id * mbr_segment_size;

			//duplicate coded block
			dup_disk_id =
			    mbr_get_dup_disk_id(mbr_code_block_id,
						mbr_segment_size);
			dup_block_no =
			    mbr_get_dup_block_no(code_disk_id,
						 mbr_code_block_id,
						 mbr_segment_size);
			dup_block_no =
			    dup_block_no + mbr_segment_id * mbr_segment_size;

			//Cache Start
			retstat =
			    cacheLayer->DiskRead(code_disk_id, buf_read,
						 size_request,
						 code_block_no * block_size);
			//Cache End

			//Calculate the code block on the Gauss Field
			int coefficient =
			    NCFS_DATA->generator_matrix[i * NCFS_DATA->mbr_m +
							mbr_native_block_id];

			for (j = 0; j < size_request; j++) {
				buf_temp[j] =
				    galois_single_multiply((unsigned char)
							   buf2[j], coefficient,
							   field_power);
			}

			for (j = 0; j < size_request; j++) {
				buf_read[j] = buf_read[j] ^ buf_temp[j];
			}

			// Cache Start
			//write coded block
			retstat =
			    cacheLayer->DiskWrite(code_disk_id, buf_read, size,
						  code_block_no * block_size);
			//write duplicate coded block
			retstat =
			    cacheLayer->DiskWrite(dup_disk_id, buf_read, size,
						  dup_block_no * block_size);
			// Cache End
		}

		free(buf_temp);
		free(buf_read);
		free(buf2);
	}

	block_written.disk_id = disk_id;
	block_written.block_no = block_no;

	return block_written;
}

/*
 * encoding_rs: Reed-Solomon code (type=3000)
 *
 * @param buf: buffer of data to be written to disk
 * @param size: size of the data buffer
 *
 * @return: assigned data block: disk id and block number.
 */
struct data_block_info CodingLayer::encoding_rs(const char *buf, int size)
{
	int retstat, disk_id, block_no, disk_total_num, data_disk_num,
	    block_size;

	int parity_block_location;

	int size_request, block_request, free_offset;
	struct data_block_info block_written;
	int i, j;
	int parity_disk_id;

	//buf_write is the data to be written to parity disks
	//buf_read is the origin data
	char *buf_temp, *buf_write, *buf_read;
	//char temp_char;
	int data_disk_coeff;

	int fault_tolerance;

	//n
	disk_total_num = NCFS_DATA->disk_total_num;

	//k
	data_disk_num = NCFS_DATA->data_disk_num;

	//m
	fault_tolerance = (disk_total_num - data_disk_num);

	block_size = NCFS_DATA->chunk_size;

	size_request = fileSystemLayer->round_to_block_size(size);
	block_request = size_request / block_size;

	//implement disk write algorithm here.
	//here use star code: stripped block allocation plus distributed parity.

	block_no = 0;
	disk_id = -1;
	free_offset = NCFS_DATA->free_offset[0];

	int tmp_free_offset = free_offset;

	//find the available data disk
	for (i = data_disk_num - 1; i >= 0; i--) {

		if (block_request <= (NCFS_DATA->free_size[i])) {

			if ((NCFS_DATA->free_offset[i] % (data_disk_num - 1)) != 0)	
			//if free_offset mode (data_disk_num-1) is nonzero, then the disk is writable
			{
				disk_id = i;
				block_no = NCFS_DATA->free_offset[i];
				tmp_free_offset = block_no;

			} else if (free_offset > (NCFS_DATA->free_offset[i]))	
			// if free_offset is smaller than free_offset in disk0, then the disk is writable
			{
				disk_id = i;
				block_no = NCFS_DATA->free_offset[i];
				tmp_free_offset = block_no;

			} else if ((i == 0) && (disk_id == -1))	
			// the special case: write disk 0 first
			{
				disk_id = i;
				block_no = NCFS_DATA->free_offset[i];
				tmp_free_offset = block_no;
			}
		}
	}

	free_offset = tmp_free_offset;

	//get block from space_list if no free block available
	if (disk_id == -1) {
		if (NCFS_DATA->space_list_head != NULL) {
			disk_id = NCFS_DATA->space_list_head->disk_id;
			block_no = NCFS_DATA->space_list_head->disk_block_no;
			fileSystemLayer->space_list_remove(disk_id, block_no);
		}
	}

	if (disk_id == -1) {
		printf("***get_data_block_no: ERROR disk_id = -1\n");
	} else {
		//temp buffer
		buf_read = (char *)malloc(sizeof(char) * size_request);
		buf_temp = (char *)malloc(sizeof(char) * size_request);
		buf_write = (char *)malloc(sizeof(char) * size_request);
		memset(buf_write, 0, size_request);

		//allocate the available data block
		NCFS_DATA->free_offset[disk_id] = block_no + block_request;
		NCFS_DATA->free_size[disk_id]
		    = NCFS_DATA->free_size[disk_id] - block_request;

		//read the original data block
		retstat = cacheLayer->DiskRead(disk_id, buf_read,
					       size_request,
					       block_no * block_size);

		//calculate the XOR of buf and buf_read
		for (j = 0; j < size_request; j++) {
			buf_read[j] = buf[j] ^ buf_read[j];
		}

		// Cache Start
		//write the data block into disk
		retstat =
		    cacheLayer->DiskWrite(disk_id, buf, size,
					  block_no * block_size);
		// Cache End

		//update the code disks one by one
		for (i = 0; i < fault_tolerance; i++) {

			memset(buf_temp, 0, size_request);

			data_disk_coeff =
			    NCFS_DATA->generator_matrix[i * data_disk_num +
							disk_id];

			for (j = 0; j < size_request; j++) {
				buf_temp[j] =
				    galois_single_multiply((unsigned char)
							   buf_read[j],
							   data_disk_coeff,
							   field_power);
			}

			//update the corresponding parity block in the appropriate location
			parity_disk_id = data_disk_num + i;

			parity_block_location = block_no * block_size;

			retstat =
			    cacheLayer->DiskRead(parity_disk_id, buf_write,
						 size_request,
						 parity_block_location);

			for (j = 0; j < size_request; j++) {

				//Calculate parity block P
				buf_write[j] = buf_write[j] ^ buf_temp[j];

			}

			retstat =
			    cacheLayer->DiskWrite(parity_disk_id, buf_write,
						  size, parity_block_location);
		}

		free(buf_temp);
		free(buf_read);
		free(buf_write);
	}

	block_written.disk_id = disk_id;
	block_written.block_no = block_no;

	return block_written;
}

/*************************************************************************
 * Decoding functions
 *************************************************************************/

/*
 * decoding_default: no decoding
 *
 * @param disk_id: disk id
 * @param buf: data buffer for reading data
 * @param size: size of the data buffer
 * @param offset: offset of data on disk
 *
 *@return: size of successful read (in bytes)
 */
int CodingLayer::decoding_default(int disk_id, char *buf, long long size,
				  long long offset)
{
	if (NCFS_DATA->disk_status[disk_id] == 0)
		return cacheLayer->DiskRead(disk_id, buf, size, offset);
	else {
		printf("Raid %d: Disk %d failed\n", NCFS_DATA->disk_raid_type,
		       disk_id);
		return -1;
	}
	AbnormalError();
	return -1;
}

/*
 * Decoding_JBOD: JBOD decode
 *
 * @param disk_id: disk id
 * @param buf: data buffer for reading data
 * @param size: size of the data buffer
 * @param offset: offset of data on disk
 *
 * @return: size of successful decoding (in bytes)
 */
int CodingLayer::decoding_jbod(int disk_id, char *buf, long long size,
			       long long offset)
{
	return decoding_default(disk_id, buf, size, offset);
}

/*
 * decoding_raid0: RAID 0 decode
 *
 * @param disk_id: disk id
 * @param buf: data buffer for reading data
 * @param size: size of the data buffer
 * @param offset: offset of data on disk
 *
 * @return: size of successful decoding (in bytes)
 */
int CodingLayer::decoding_raid0(int disk_id, char *buf, long long size,
				long long offset)
{
	return decoding_default(disk_id, buf, size, offset);
}

/*
 * decoding_raid1: RAID 1 decode
 *
 * @param disk_id: disk id
 * @param buf: data buffer for reading data
 * @param size: size of the data buffer
 * @param offset: offset of data on disk
 *
 * @return: size of successful decoding (in bytes)
 */
int CodingLayer::decoding_raid1(int disk_id, char *buf, long long size,
				long long offset)
{
	if (NCFS_DATA->disk_status[disk_id] == 0)
		return cacheLayer->DiskRead(disk_id, buf, size, offset);
	else {
		int mirror_disk_id = NCFS_DATA->disk_total_num - disk_id - 1;
		if (NCFS_DATA->disk_status[mirror_disk_id] == 0) {
			return cacheLayer->DiskRead(mirror_disk_id, buf, size,
						    offset);
		} else {
			printf("Raid 1 both disk %d and mirror disk %d\n",
			       disk_id, mirror_disk_id);
			return -1;
		}
	}
	AbnormalError();
	return -1;
}

/*
 * decoding_raid4: RAID 4 decode
 *
 * @param disk_id: disk id               
 * @param buf: data buffer for reading data
 * @param size: size of the data buffer
 * @param offset: offset of data on disk
 *
 * @return: size of successful decoding (in bytes)
 */
int CodingLayer::decoding_raid4(int disk_id, char *buf, long long size,
				long long offset)
{
	if (NCFS_DATA->disk_status[disk_id] == 0)
		return cacheLayer->DiskRead(disk_id, buf, size, offset);
	else {
		int retstat;
		char *temp_buf;
		int i;
		long long j;
		temp_buf = (char *)malloc(sizeof(char) * size);
		memset(temp_buf, 0, size);
		memset(buf, 0, size);
		int *inttemp_buf = (int *)temp_buf;
		int *intbuf = (int *)buf;
		for (i = 0; i < NCFS_DATA->disk_total_num; ++i) {
			if (i != disk_id) {
				if (NCFS_DATA->disk_status[i] != 0) {
					printf
					    ("Raid 4 both disk %d and %d are failed\n",
					     disk_id, i);
					return -1;
				}
				retstat =
				    cacheLayer->DiskRead(i, temp_buf, size,
							 offset);
				for (j = 0;
				     j <
				     (long long)(size * sizeof(char) /
						 sizeof(int)); ++j)
					intbuf[j] = intbuf[j] ^ inttemp_buf[j];
			}
		}
		free(temp_buf);
		return size;
	}
	AbnormalError();
	return -1;
}

/*
 * decoding_raid5: RAID 5 decode
 *
 * @param disk_id: disk id               
 * @param buf: data buffer for reading data
 * @param size: size of the data buffer
 * @param offset: offset of data on disk
 *
 * @return: size of successful decoding (in bytes)
 */
int CodingLayer::decoding_raid5(int disk_id, char *buf, long long size,
				long long offset)
{
	int retstat;
	struct timeval t1, t2, t3;
	double duration;

	if (NCFS_DATA->disk_status[disk_id] == 0) {
		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t1, NULL);
		}

		retstat = cacheLayer->DiskRead(disk_id, buf, size, offset);

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t2, NULL);
		}

		return retstat;
	} else {
		char *temp_buf;
		int i;
		long long j;
		temp_buf = (char *)malloc(sizeof(char) * size);
		memset(temp_buf, 0, size);
		memset(buf, 0, size);
		int *inttemp_buf = (int *)temp_buf;
		int *intbuf = (int *)buf;
		for (i = 0; i < NCFS_DATA->disk_total_num; ++i) {
			if (i != disk_id) {
				if (NCFS_DATA->disk_status[i] != 0) {
					printf
					    ("Raid 5 both disk %d and %d are failed\n",
					     disk_id, i);
					free(temp_buf);
					return -1;
				}

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t1, NULL);
				}

				retstat =
				    cacheLayer->DiskRead(i, temp_buf, size,
							 offset);

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t2, NULL);
				}

				for (j = 0;
				     j <
				     (long long)(size * sizeof(char) /
						 sizeof(int)); ++j)
					intbuf[j] = intbuf[j] ^ inttemp_buf[j];

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t3, NULL);

					duration = (t3.tv_sec - t2.tv_sec) +
					    (t3.tv_usec -
					     t2.tv_usec) / 1000000.0;
					NCFS_DATA->decoding_time += duration;
				}
			}
		}
		free(temp_buf);
		return size;
	}
	AbnormalError();
	return -1;
}

/*
 * decoding_raid6: RAID 6 decode
 *
 * @param disk_id: disk id
 * @param buf: data buffer for reading data
 * @param size: size of the data buffer
 * @param offset: offset of data on disk
 *
 * @return: size of successful decoding (in bytes)
 */
int CodingLayer::decoding_raid6(int disk_id, char *buf, long long size,
				long long offset)
{

	int retstat;
	char *temp_buf;
	int i;
	long long j;
	int disk_failed_no;
	int disk_another_failed_id;	//id of second failed disk

	int code_disk_id, parity_disk_id;
	int block_size;
	long long block_no;
	int disk_total_num;
	int data_disk_coeff;
	char temp_char;

	int g1, g2, g12;
	char *P_temp;
	char *Q_temp;

	struct timeval t1, t2, t3;
	double duration;

	temp_buf = (char *)malloc(sizeof(char) * size);
	memset(temp_buf, 0, size);

	P_temp = (char *)malloc(sizeof(char) * size);
	memset(P_temp, 0, size);

	Q_temp = (char *)malloc(sizeof(char) * size);
	memset(Q_temp, 0, size);

	memset(buf, 0, size);
	int *inttemp_buf = (int *)temp_buf;
	int *intbuf = (int *)buf;

	disk_total_num = NCFS_DATA->disk_total_num;
	block_size = NCFS_DATA->chunk_size;
	block_no = offset / block_size;

	code_disk_id = disk_total_num - 1 - (block_no % disk_total_num);
	parity_disk_id = disk_total_num - 1 - ((block_no + 1) % disk_total_num);
	//code_disk_id = disk_total_num - 1;
	//parity_disk_id = disk_total_num - 2;

	//target disk is good
	if (NCFS_DATA->disk_status[disk_id] == 0) {
		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t1, NULL);
		}

		retstat = cacheLayer->DiskRead(disk_id, buf, size, offset);

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t2, NULL);
		}

		free(temp_buf);
		free(P_temp);
		free(Q_temp);

		return retstat;
	} else { //target disk is failed
		//check the number of failed disks
		disk_failed_no = 0;
		for (i = 0; i < disk_total_num; i++) {
			if (NCFS_DATA->disk_status[i] == 1) {
				(disk_failed_no)++;
				if (i != disk_id) {
					disk_another_failed_id = i;
				}
			}
		}

		if (((disk_failed_no == 1) && (disk_id != code_disk_id)) ||
		    ((disk_failed_no == 2)
		     && (disk_another_failed_id == code_disk_id))) {
			//case of single non-Q disk fail (D or P), or two-disk (data disk + Q disk) fail (D + Q)
			for (i = 0; i < disk_total_num; ++i) {
				if ((i != disk_id) && (i != code_disk_id)) {
					if (NCFS_DATA->disk_status[i] != 0) {
						printf
						    ("Raid 6 both disk %d and %d are failed\n",
						     disk_id, i);
						return -1;
					}

					if (NCFS_DATA->run_experiment == 1) {
						gettimeofday(&t1, NULL);
					}

					retstat =
					    cacheLayer->DiskRead(i, temp_buf,
								 size, offset);

					if (NCFS_DATA->run_experiment == 1) {
						gettimeofday(&t2, NULL);
					}

					for (j = 0;
					     j <
					     (long long)(size * sizeof(char) /
							 sizeof(int)); ++j) {
						intbuf[j] =
						    intbuf[j] ^ inttemp_buf[j];
					}

					if (NCFS_DATA->run_experiment == 1) {
						gettimeofday(&t3, NULL);

						duration =
						    (t3.tv_sec - t2.tv_sec) +
						    (t3.tv_usec -
						     t2.tv_usec) / 1000000.0;
						NCFS_DATA->decoding_time +=
						    duration;
					}
				}
			}
		} else if (((disk_failed_no == 1) && (disk_id == code_disk_id))
			   || ((disk_failed_no == 2)
			       && (disk_id == code_disk_id)
			       && (disk_another_failed_id == parity_disk_id))) {
			//case of "single Q disk fails" (Q) or "Q + P disk failure" (Q + P)

			//Calculate Q
			for (i = 0; i < disk_total_num; i++) {
				if ((i != parity_disk_id)
				    && (i != code_disk_id)) {

					if (NCFS_DATA->run_experiment == 1) {
						gettimeofday(&t1, NULL);
					}
					//Cache Start
					retstat =
					    cacheLayer->DiskRead(i, temp_buf,
								 size, offset);
					//Cache End

					if (NCFS_DATA->run_experiment == 1) {
						gettimeofday(&t2, NULL);
					}

					for (j = 0; j < size; j++) {
						//calculate the coefficient of the data block
						data_disk_coeff = i;

						if (i > code_disk_id) {
							(data_disk_coeff)--;
						}
						if (i > parity_disk_id) {
							(data_disk_coeff)--;
						}
						data_disk_coeff =
						    disk_total_num - 3 -
						    data_disk_coeff;
						data_disk_coeff =
						    gf_get_coefficient
						    (data_disk_coeff,
						     field_power);

						//calculate code block Q
						buf[j] = buf[j] ^
						    (char)gf_mul((unsigned char)
								 temp_buf[j],
								 data_disk_coeff,
								 field_power);
					}

					if (NCFS_DATA->run_experiment == 1) {
						gettimeofday(&t3, NULL);

						duration =
						    (t3.tv_sec - t2.tv_sec) +
						    (t3.tv_usec -
						     t2.tv_usec) / 1000000.0;
						NCFS_DATA->decoding_time +=
						    duration;
					}
				}
			}
		} else if (disk_failed_no == 2) {
			//case of two-disk fail (data disk + non Q disk) (disk_id and disk_second_id)
			//compute Q' to restore data

			if ((disk_id == parity_disk_id)
			    && (disk_another_failed_id != code_disk_id)) {
				//case of P disk + data disk fail (P + D)

				//calculate Q'
				for (i = 0; i < disk_total_num; i++) {
					if ((i != disk_id)
					    && (i != disk_another_failed_id)
					    && (i != parity_disk_id)
					    && (i != code_disk_id)) {

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t1, NULL);
						}

						retstat =
						    cacheLayer->DiskRead(i,
									 temp_buf,
									 size,
									 offset);

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t2, NULL);
						}
						//calculate the coefficient of the data block
						data_disk_coeff = i;

						if (i > code_disk_id) {
							(data_disk_coeff)--;
						}
						if (i > parity_disk_id) {
							(data_disk_coeff)--;
						}
						data_disk_coeff =
						    disk_total_num - 3 -
						    data_disk_coeff;
						data_disk_coeff =
						    gf_get_coefficient
						    (data_disk_coeff,
						     field_power);

						for (j = 0; j < size; j++) {
							buf[j] = buf[j] ^
							    (char)
							    gf_mul((unsigned
								    char)
								   temp_buf[j],
								   data_disk_coeff,
								   field_power);
						}

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t3, NULL);

							duration =
							    (t3.tv_sec -
							     t2.tv_sec) +
							    (t3.tv_usec -
							     t2.tv_usec) /
							    1000000.0;
							NCFS_DATA->
							    decoding_time +=
							    duration;
						}
					}
				}

				//calculate Q xor Q'

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t1, NULL);
				}

				retstat =
				    cacheLayer->DiskRead(code_disk_id, temp_buf,
							 size, offset);

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t2, NULL);
				}

				for (j = 0; j < size; j++) {
					buf[j] = buf[j] ^ temp_buf[j];
				}

				//calculate the coefficient of the data block
				data_disk_coeff = disk_id;

				if (disk_id > code_disk_id) {
					(data_disk_coeff)--;
				}
				if (disk_id > parity_disk_id) {
					(data_disk_coeff)--;
				}
				data_disk_coeff =
				    disk_total_num - 3 - data_disk_coeff;
				data_disk_coeff =
				    gf_get_coefficient(data_disk_coeff,
						       field_power);

				//decode the origianl data block
				for (j = 0; j < size; j++) {
					temp_char = buf[j];
					buf[j] =
					    (char)gf_div((unsigned char)
							 temp_char,
							 data_disk_coeff,
							 field_power);
				}

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t3, NULL);

					duration = (t3.tv_sec - t2.tv_sec) +
					    (t3.tv_usec -
					     t2.tv_usec) / 1000000.0;
					NCFS_DATA->decoding_time += duration;
				}
				//find P
				for (i = 0; i < disk_total_num; ++i) {
					if ((i != disk_id)
					    && (i != code_disk_id)
					    && (i != disk_another_failed_id)) {
						if (NCFS_DATA->disk_status[i] !=
						    0) {
							printf
							    ("Raid 6 both disk %d and %d are failed\n",
							     disk_id, i);
							return -1;
						}

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t1, NULL);
						}

						retstat =
						    cacheLayer->DiskRead(i,
									 temp_buf,
									 size,
									 offset);

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t2, NULL);
						}

						for (j = 0;
						     j <
						     (long long)(size *
								 sizeof(char) /
								 sizeof(int));
						     ++j) {
							intbuf[j] =
							    intbuf[j] ^
							    inttemp_buf[j];
						}

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t3, NULL);

							duration =
							    (t3.tv_sec -
							     t2.tv_sec) +
							    (t3.tv_usec -
							     t2.tv_usec) /
							    1000000.0;
							NCFS_DATA->
							    decoding_time +=
							    duration;
						}
					}
				}
			} else if ((disk_id == code_disk_id)
				   && (disk_another_failed_id !=
				       parity_disk_id)) {
				//case of Q disk + data disk fail (Q + D)

				//find D from P
				for (i = 0; i < disk_total_num; ++i) {
					if ((i != disk_id)
					    && (i != disk_another_failed_id)) {
						if (NCFS_DATA->disk_status[i] !=
						    0) {
							printf
							    ("Raid 6 both disk %d and %d are failed\n",
							     disk_id, i);
							return -1;
						}

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t1, NULL);
						}

						retstat =
						    cacheLayer->DiskRead(i,
									 temp_buf,
									 size,
									 offset);

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t2, NULL);
						}

						for (j = 0;
						     j <
						     (long long)(size *
								 sizeof(char) /
								 sizeof(int));
						     ++j) {
							intbuf[j] =
							    intbuf[j] ^
							    inttemp_buf[j];
						}

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t3, NULL);

							duration =
							    (t3.tv_sec -
							     t2.tv_sec) +
							    (t3.tv_usec -
							     t2.tv_usec) /
							    1000000.0;
							NCFS_DATA->
							    decoding_time +=
							    duration;
						}
					}
				}

				//calculate Q
				for (i = 0; i < disk_total_num; i++) {
					if ((i != parity_disk_id)
					    && (i != code_disk_id)) {
						if (i != disk_another_failed_id) {
							if (NCFS_DATA->
							    run_experiment ==
							    1) {
								gettimeofday
								    (&t1, NULL);
							}
							//Cache Start
							retstat =
							    cacheLayer->
							    DiskRead(i,
								     temp_buf,
								     size,
								     offset);
							//Cache End

							if (NCFS_DATA->
							    run_experiment ==
							    1) {
								gettimeofday
								    (&t2, NULL);
							}
						} else {	//use the recovered D
							for (j = 0; j < size;
							     j++) {
								temp_buf[j] =
								    buf[j];
							}
						}

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t1, NULL);
						}

						for (j = 0; j < size; j++) {
							//calculate the coefficient of the data block
							data_disk_coeff = i;

							if (i > code_disk_id) {
								(data_disk_coeff)--;
							}
							if (i > parity_disk_id) {
								(data_disk_coeff)--;
							}
							data_disk_coeff =
							    disk_total_num - 3 -
							    data_disk_coeff;
							data_disk_coeff =
							    gf_get_coefficient
							    (data_disk_coeff,
							     field_power);

							//calculate code block Q
							Q_temp[j] = Q_temp[j] ^
							    (char)
							    gf_mul((unsigned
								    char)
								   temp_buf[j],
								   data_disk_coeff,
								   field_power);
						}

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t2, NULL);
						}
					}
				}

				for (j = 0; j < size; j++) {
					buf[j] = Q_temp[j];
				}
			} else if (disk_another_failed_id == parity_disk_id) {
				//case of data disk + P disk fail (D + P)

				//calculate Q'
				for (i = 0; i < disk_total_num; i++) {
					if ((i != disk_id)
					    && (i != disk_another_failed_id)
					    && (i != parity_disk_id)
					    && (i != code_disk_id)) {

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t1, NULL);
						}

						retstat =
						    cacheLayer->DiskRead(i,
									 temp_buf,
									 size,
									 offset);

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t2, NULL);
						}
						//calculate the coefficient of the data block
						data_disk_coeff = i;

						if (i > code_disk_id) {
							(data_disk_coeff)--;
						}
						if (i > parity_disk_id) {
							(data_disk_coeff)--;
						}
						data_disk_coeff =
						    disk_total_num - 3 -
						    data_disk_coeff;
						data_disk_coeff =
						    gf_get_coefficient
						    (data_disk_coeff,
						     field_power);

						for (j = 0; j < size; j++) {
							temp_char = buf[j];
							buf[j] = buf[j] ^
							    (char)
							    gf_mul((unsigned
								    char)
								   temp_buf[j],
								   data_disk_coeff,
								   field_power);
						}

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t3, NULL);

							duration =
							    (t3.tv_sec -
							     t2.tv_sec) +
							    (t3.tv_usec -
							     t2.tv_usec) /
							    1000000.0;
							NCFS_DATA->
							    decoding_time +=
							    duration;
						}
					}
				}

				//calculate Q xor Q'

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t1, NULL);
				}

				retstat =
				    cacheLayer->DiskRead(code_disk_id, temp_buf,
							 size, offset);

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t2, NULL);
				}

				for (j = 0; j < size; j++) {
					buf[j] = buf[j] ^ temp_buf[j];
				}

				//calculate the coefficient of the data block
				data_disk_coeff = disk_id;

				if (disk_id > code_disk_id) {
					(data_disk_coeff)--;
				}
				if (disk_id > parity_disk_id) {
					(data_disk_coeff)--;
				}
				data_disk_coeff =
				    disk_total_num - 3 - data_disk_coeff;
				data_disk_coeff =
				    gf_get_coefficient(data_disk_coeff,
						       field_power);

				//decode the origianl data block
				for (j = 0; j < size; j++) {
					temp_char = buf[j];
					buf[j] =
					    (char)gf_div((unsigned char)
							 temp_char,
							 data_disk_coeff,
							 field_power);
				}

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t3, NULL);

					duration = (t3.tv_sec - t2.tv_sec) +
					    (t3.tv_usec -
					     t2.tv_usec) / 1000000.0;
					NCFS_DATA->decoding_time += duration;
				}
				//return retstat;
			} else {
				// two data disk fail (D + D)

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t1, NULL);
				}
				//calculate g1
				g1 = disk_id;
				if (g1 > code_disk_id) {
					(g1)--;
				}
				if (g1 > parity_disk_id) {
					(g1)--;
				}
				g1 = disk_total_num - 3 - g1;
				g1 = gf_get_coefficient(g1, field_power);

				//calculate g2
				g2 = disk_another_failed_id;
				if (g2 > code_disk_id) {
					(g2)--;
				}
				if (g2 > parity_disk_id) {
					(g2)--;
				}
				g2 = disk_total_num - 3 - g2;
				g2 = gf_get_coefficient(g2, field_power);

				//calculate g12
				g12 = g1 ^ g2;

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t2, NULL);

					duration = (t2.tv_sec - t1.tv_sec) +
					    (t2.tv_usec -
					     t1.tv_usec) / 1000000.0;
					NCFS_DATA->decoding_time += duration;
				}

				for (i = 0; i < disk_total_num; i++) {
					if ((i != disk_id)
					    && (i != disk_another_failed_id)
					    && (i != parity_disk_id)
					    && (i != code_disk_id)) {
						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t1, NULL);
						}

						retstat =
						    cacheLayer->DiskRead(i,
									 temp_buf,
									 size,
									 offset);

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t2, NULL);
						}

						for (j = 0; j < size; j++) {
							P_temp[j] =
							    P_temp[j] ^
							    temp_buf[j];
						}

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t3, NULL);

							duration =
							    (t3.tv_sec -
							     t2.tv_sec) +
							    (t3.tv_usec -
							     t2.tv_usec) /
							    1000000.0;
							NCFS_DATA->
							    decoding_time +=
							    duration;
						}
					}
				}

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t1, NULL);
				}

				retstat =
				    cacheLayer->DiskRead(parity_disk_id,
							 temp_buf, size,
							 offset);

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t2, NULL);
				}

				for (j = 0; j < size; j++) {
					P_temp[j] = P_temp[j] ^ temp_buf[j];
					//P_temp = P' xor P
				}

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t3, NULL);

					duration = (t3.tv_sec - t2.tv_sec) +
					    (t3.tv_usec -
					     t2.tv_usec) / 1000000.0;
					NCFS_DATA->decoding_time += duration;
				}
				for (i = 0; i < disk_total_num; i++) {
					if (((i != disk_id)
					     && (i != disk_another_failed_id))
					    && (i != parity_disk_id)
					    && (i != code_disk_id)) {

						//calculate the coefficient of the data block
						data_disk_coeff = i;

						if (i > code_disk_id) {
							(data_disk_coeff)--;
						}
						if (i > parity_disk_id) {
							(data_disk_coeff)--;
						}
						data_disk_coeff =
						    disk_total_num - 3 -
						    data_disk_coeff;
						data_disk_coeff =
						    gf_get_coefficient
						    (data_disk_coeff,
						     field_power);

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t1, NULL);
						}

						retstat =
						    cacheLayer->DiskRead(i,
									 temp_buf,
									 size,
									 offset);

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t2, NULL);
						}

						for (j = 0; j < size; j++) {
							temp_char = Q_temp[j];
							Q_temp[j] = temp_char ^
							    (char)
							    gf_mul((unsigned
								    char)
								   temp_buf[j],
								   data_disk_coeff,
								   field_power);
						}

						if (NCFS_DATA->run_experiment ==
						    1) {
							gettimeofday(&t3, NULL);

							duration =
							    (t3.tv_sec -
							     t2.tv_sec) +
							    (t3.tv_usec -
							     t2.tv_usec) /
							    1000000.0;
							NCFS_DATA->
							    decoding_time +=
							    duration;
						}
					}

					//calculate D

					if (NCFS_DATA->run_experiment == 1) {
						gettimeofday(&t1, NULL);
					}

					retstat =
					    cacheLayer->DiskRead(code_disk_id,
								 temp_buf, size,
								 offset);

					if (NCFS_DATA->run_experiment == 1) {
						gettimeofday(&t2, NULL);
					}

					for (j = 0; j < size; j++) {
						temp_char =
						    (char)(gf_mul
							   (g2,
							    (unsigned char)
							    P_temp[j],
							    field_power)
							   ^ Q_temp[j] ^
							   temp_buf[j]);
						buf[j] =
						    (char)gf_div((unsigned char)
								 temp_char, g12,
								 field_power);
					}

					if (NCFS_DATA->run_experiment == 1) {
						gettimeofday(&t3, NULL);

						duration =
						    (t3.tv_sec - t2.tv_sec) +
					    (t3.tv_usec -
						     t2.tv_usec) / 1000000.0;
						NCFS_DATA->decoding_time +=
						    duration;
					}
				}
			}
		} else {
			printf
			    ("Raid 6 number of failed disks larger than 2.\n");
			return -1;
		}

		free(temp_buf);
		free(P_temp);
		free(Q_temp);
		return size;
	}
	AbnormalError();

	return -1;
}

//Add by Dongsheng Wei on Jan. 16, 2014 begin.
int CodingLayer::decoding_mdr_I(int disk_id, char *buf, long long size,
				long long offset)
{
	int retstat;
	char *temp_buf;

	int disk_failed_no;
	int disk_another_failed_id;	//id of second failed disk

	int q_disk_id, p_disk_id;
	int block_size;
	long long block_no;
	int disk_total_num;
	int data_disk_coeff;
	char temp_char;

	int g1, g2, g12;
	char *P_temp;
	char *Q_temp;

	struct timeval t1, t2, t3;
	double duration;

	memset(buf, 0, size);

	temp_buf = (char *)malloc(sizeof(char) * size);
	memset(temp_buf, 0, size);

	vector<vector<int> > repair_q_blocks_no;

	disk_total_num = NCFS_DATA->disk_total_num;
	block_size = NCFS_DATA->chunk_size;
	block_no = offset / block_size;

	q_disk_id = disk_total_num - 1;
	p_disk_id = disk_total_num - 2;

	if (NCFS_DATA->disk_status[disk_id] == 0) {
		retstat = cacheLayer->DiskRead(disk_id, buf, size, offset);
		return retstat;
	}
	else{
		//check the number of failed disks
		disk_failed_no = 0;
		for (int i = 0; i < disk_total_num; i++) {
			if (NCFS_DATA->disk_status[i] == 1) {
				(disk_failed_no)++;
				if (i != disk_id) {
					disk_another_failed_id = i;
				}
			}
		}

		int strip_num = block_no / strip_size;
		int strip_offset = block_no % strip_size;
		
		if(disk_failed_no == 1){ //fail disk id = disk_id
			if((disk_id >= 0) && (disk_id < disk_total_num-1)){
				//fail disk(disk_id) is data disk or P disk	

				//find the stripe index which is must be read
				if(mdr_I_one_dpDisk_fail_bool_v == false){
					mdr_I_one_dpDisk_fail_bool_v = true;
					mdr_I_one_dpDisk_fail_stripeIndex = 
						mdr_I_repair_dpDisk_stripeIndexs(disk_id, NCFS_DATA->data_disk_num);
				}
				
				// cout<<"mdr_I_one_dpDisk_fail_stripeIndex:\n";
				// print_ivec(mdr_I_one_dpDisk_fail_stripeIndex);

				set<int> stripeIndexs_set;
				stripeIndexs_set.insert(
					mdr_I_one_dpDisk_fail_stripeIndex.begin(), 
					mdr_I_one_dpDisk_fail_stripeIndex.end()
					);

				//if the needed blk is in the stripeIndexs
				if(stripeIndexs_set.find(strip_offset) 
					!= stripeIndexs_set.end()){

					for(int i = 0; i < disk_total_num-1; i++){
						if(i != disk_id){
							retstat = cacheLayer->DiskRead(i, temp_buf, size, offset);
							for(long long j = 0; j < size; j++){
								buf[j] = buf[j] ^ temp_buf[j];
							}
						}
					}
				}
				else{// if the neede blk is not in the stripeIndexs
					if(mdr_I_one_dpDisk_fail_bool_m == false){
						mdr_I_one_dpDisk_fail_bool_m = true;

						mdr_I_one_dpDisk_fail_nonStripeIndex = 
								mdr_I_repair_dpDisk_nonstripeIndexs_blocks_no(disk_id, 
							 							mdr_I_one_dpDisk_fail_stripeIndex);
					}					

					// cout<<"mdr_I_one_dpDisk_fail_nonStripeIndex\n";
					// print_ivmap(mdr_I_one_dpDisk_fail_nonStripeIndex, mdr_I_one_dpDisk_fail_stripeIndex);

					vector<vector<int> > iivec = mdr_I_one_dpDisk_fail_nonStripeIndex[strip_offset];

					//cout<<"iivec:\n";
					//print_iivec(iivec);
					int iivec_size = iivec.size();
					//cout<<"iivec_size = "<<iivec_size<<endl;

					if(iivec_size != NCFS_DATA->disk_total_num){
						printf("error: repair reading blks num\n");
						exit(1);
					}

					int debug_count = 0;

					for(int i = 0; i < iivec_size; i++){
						if(!iivec[i].empty()){	
							//the needed blk should be repaired firstly
							if(i == disk_id){
								int ivec_size = iivec[i].size();
								for(int i2 = 0; i2 < ivec_size; i2++){
									int i2_blk_num = iivec[i][i2]+strip_num*strip_size;

									for(int ii = 0; ii < disk_total_num-1; ii++){
										if(ii != i){
											//printf("debug_count = %d, [disk_id, block_no] = [%d, %d]\n", ++debug_count, ii, i2_blk_num);
											retstat = cacheLayer->DiskRead(ii, temp_buf, size, i2_blk_num*block_size);
											for(long long j = 0; j < size; j++){
												buf[j] = buf[j] ^ temp_buf[j];
											}
										}
									}
								}
							}
					
							//all the needed blks can be read directly
							if(i != disk_id){
								int ivec_size = iivec[i].size();
								for(int ii = 0; ii < ivec_size; ii++){
									int blk_num = iivec[i][ii]+strip_num * strip_size;

									//printf("debug_count = %d, [disk_id, block_no] = [%d, %d]\n", ++debug_count, i, blk_num);
		
									retstat = cacheLayer->DiskRead(i, temp_buf, size, blk_num * block_size);

									for(long long j = 0; j < size; j++){
										buf[j] = buf[j] ^ temp_buf[j];
									}
								}
							}
						}
					}
				}
			}
			else if(disk_id = disk_total_num - 1){
				//fail disk(disk_id) is Q disk
				//TODO:	
			}
		}
		else{
			printf("MDR_I number of failed disks is larger than 1.\n");
			return -1;
		}

		free(temp_buf);
		return size;

	}
	AbnormalError();
	return -1;
}


int CodingLayer::decoding_mdr_I_recover(int disk_id, char *buf, long long size,
				long long offset, char*** pread_stripes, bool** isInbuf)
{
	int retstat;
	char *temp_buf;

	int disk_failed_no;
	int disk_another_failed_id;	//id of second failed disk

	int q_disk_id, p_disk_id;
	int block_size;
	long long block_no;
	int disk_total_num;
	int data_disk_coeff;
	char temp_char;

	int g1, g2, g12;
	char *P_temp;
	char *Q_temp;

	struct timeval t1, t2, t3;
	double duration;

	memset(buf, 0, size);

	temp_buf = (char *)malloc(sizeof(char) * size);
	memset(temp_buf, 0, size);

	vector<vector<int> > repair_q_blocks_no;

	disk_total_num = NCFS_DATA->disk_total_num;
	block_size = NCFS_DATA->chunk_size;
	block_no = offset / block_size;

	q_disk_id = disk_total_num - 1;
	p_disk_id = disk_total_num - 2;

	if (NCFS_DATA->disk_status[disk_id] == 0) {

		retstat = cacheLayer->DiskRead(disk_id, buf, size, offset);
		return retstat;
	}
	else{
		//check the number of failed disks
		disk_failed_no = 0;
		for (int i = 0; i < disk_total_num; i++) {
			if (NCFS_DATA->disk_status[i] == 1) {
				(disk_failed_no)++;
				if (i != disk_id) {
					disk_another_failed_id = i;
				}
			}
		}

		int strip_num = block_no / strip_size;
		int strip_offset = block_no % strip_size;
		

		if(disk_failed_no == 1){ //fail disk id = disk_id
			if((disk_id >= 0) && (disk_id < disk_total_num-1)){
				//fail disk(disk_id) is data disk or P disk	

				//find the stripe indexs which is must be read
				if(mdr_I_one_dpDisk_fail_bool_v == false){
					mdr_I_one_dpDisk_fail_bool_v = true;
					mdr_I_one_dpDisk_fail_stripeIndex = 
						mdr_I_repair_dpDisk_stripeIndexs(disk_id, NCFS_DATA->data_disk_num);
				}

				//find the block indexs which must be repaired by the stripes in the buf
				if(mdr_I_one_dpDisk_fail_bool_m == false){
					mdr_I_one_dpDisk_fail_bool_m = true;

					mdr_I_one_dpDisk_fail_nonStripeIndex = 
							mdr_I_repair_dpDisk_nonstripeIndexs_blocks_no(disk_id, 
						 							mdr_I_one_dpDisk_fail_stripeIndex);
				}				

				//read the essencial stripes into the pread_stripes
				int s_size = mdr_I_one_dpDisk_fail_stripeIndex.size();
				for(int i = 0; i < s_size; i++){
					int blk_num = strip_num*strip_size + mdr_I_one_dpDisk_fail_stripeIndex[i];
					for(int j = 0; j < disk_total_num; j++){
						if(j != disk_id){
							retstat = cacheLayer->DiskRead(j, pread_stripes[i][j], block_size, blk_num*block_size);
						}
					}
				}

				//repair the blk in the buf strips
				for(int i = 0; i < s_size; i++){
					for(int j = 0; j < disk_total_num; j++){
						if(j != disk_id){
							for(long long j2 = 0; j2 < block_size; j2++){
								pread_stripes[i][disk_id][j2] ^= pread_stripes[i][j][j2];
							}
						}
					}
					int buf_offset = mdr_I_one_dpDisk_fail_stripeIndex[i];
					strncpy(buf+buf_offset*block_size, pread_stripes[i][disk_id], block_size);
				}

				//repair other blks in the failed disk
				





				/////////////////////////////////////////////////////////////////////////





				set<int> stripeIndexs_set;
				stripeIndexs_set.insert(
					mdr_I_one_dpDisk_fail_stripeIndex.begin(), 
					mdr_I_one_dpDisk_fail_stripeIndex.end()
					);

				//if the needed blk is in the stripeIndexs
				if(stripeIndexs_set.find(strip_offset) 
					!= stripeIndexs_set.end()){

					for(int i = 0; i < disk_total_num-1; i++){
						if(i != disk_id){
							retstat = cacheLayer->DiskRead(i, temp_buf, size, offset);
							for(long long j = 0; j < size; j++){
								buf[j] = buf[j] ^ temp_buf[j];
							}
						}
					}
				}
				else{// if the neede blk is not in the stripeIndexs
					if(mdr_I_one_dpDisk_fail_bool_m == false){
						mdr_I_one_dpDisk_fail_bool_m = true;

						mdr_I_one_dpDisk_fail_nonStripeIndex = 
								mdr_I_repair_dpDisk_nonstripeIndexs_blocks_no(disk_id, 
							 							mdr_I_one_dpDisk_fail_stripeIndex);
					}					

					// cout<<"mdr_I_one_dpDisk_fail_nonStripeIndex\n";
					// print_ivmap(mdr_I_one_dpDisk_fail_nonStripeIndex, mdr_I_one_dpDisk_fail_stripeIndex);

					vector<vector<int> > iivec = mdr_I_one_dpDisk_fail_nonStripeIndex[strip_offset];

					//cout<<"iivec:\n";
					//print_iivec(iivec);
					int iivec_size = iivec.size();
					//cout<<"iivec_size = "<<iivec_size<<endl;

					if(iivec_size != NCFS_DATA->disk_total_num){
						printf("error: repair reading blks num\n");
						exit(1);
					}

					int debug_count = 0;

					for(int i = 0; i < iivec_size; i++){
						if(!iivec[i].empty()){	
							//the needed blk should be repaired firstly
							if(i == disk_id){
								int ivec_size = iivec[i].size();
								for(int i2 = 0; i2 < ivec_size; i2++){
									int i2_blk_num = iivec[i][i2]+strip_num*strip_size;

									for(int ii = 0; ii < disk_total_num-1; ii++){
										if(ii != i){
											//printf("debug_count = %d, [disk_id, block_no] = [%d, %d]\n", ++debug_count, ii, i2_blk_num);
											retstat = cacheLayer->DiskRead(ii, temp_buf, size, i2_blk_num*block_size);
											for(long long j = 0; j < size; j++){
												buf[j] = buf[j] ^ temp_buf[j];
											}
										}
									}
								}
							}
					
							//all the needed blks can be read directly
							if(i != disk_id){
								int ivec_size = iivec[i].size();
								for(int ii = 0; ii < ivec_size; ii++){
									int blk_num = iivec[i][ii]+strip_num * strip_size;

									//printf("debug_count = %d, [disk_id, block_no] = [%d, %d]\n", ++debug_count, i, blk_num);
		
									retstat = cacheLayer->DiskRead(i, temp_buf, size, blk_num * block_size);

									for(long long j = 0; j < size; j++){
										buf[j] = buf[j] ^ temp_buf[j];
									}
								}
							}
						}
					}
				}
			}
			else if(disk_id = disk_total_num - 1){
				//fail disk(disk_id) is Q disk
				//TODO:	
			}
		}
		else{
			printf("MDR_I number of failed disks is larger than 1.\n");
			return -1;
		}

		free(temp_buf);
		return size;

	}
	AbnormalError();
	return -1;
}


int CodingLayer::decoding_raid5_noRotate(int disk_id, char *buf, long long size,
				long long offset)
{
	//RAID 5 with no rotate
	int retstat;
	struct timeval t1, t2, t3;
	double duration;

	if (NCFS_DATA->disk_status[disk_id] == 0) {
		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t1, NULL);
		}

		retstat = cacheLayer->DiskRead(disk_id, buf, size, offset);

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t2, NULL);
		}

		return retstat;
	} else {
		char *temp_buf;
		int i;
		long long j;
		temp_buf = (char *)malloc(sizeof(char) * size);
		memset(temp_buf, 0, size);
		memset(buf, 0, size);
		int *inttemp_buf = (int *)temp_buf;
		int *intbuf = (int *)buf;
		for (i = 0; i < NCFS_DATA->disk_total_num; ++i) {
			if (i != disk_id) {
				if (NCFS_DATA->disk_status[i] != 0) {
					printf
					    ("Raid 5 both disk %d and %d are failed\n",
					     disk_id, i);
					free(temp_buf);
					return -1;
				}

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t1, NULL);
				}

				retstat =
				    cacheLayer->DiskRead(i, temp_buf, size,
							 offset);

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t2, NULL);
				}

				for (j = 0;
				     j <
				     (long long)(size * sizeof(char) /
						 sizeof(int)); ++j)
					intbuf[j] = intbuf[j] ^ inttemp_buf[j];

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t3, NULL);

					duration = (t3.tv_sec - t2.tv_sec) +
					    (t3.tv_usec -
					     t2.tv_usec) / 1000000.0;
					NCFS_DATA->decoding_time += duration;
				}
			}
		}
		free(temp_buf);
		return size;
	}
	AbnormalError();
	return -1;
}

int CodingLayer::decoding_raid6_noRotate(int disk_id, char *buf, long long size,
				long long offset){
	//this is wrong!!
	return decoding_raid6(disk_id, buf, size, offset);
}
//Add by Dongsheng Wei on Jan. 16, 2014 end.


/*
 * decoding_mbr: MBR decode
 *
 * @param disk_id: disk id
 * @param buf: data buffer for reading data
 * @param size: size of the data buffer
 * @param offset: offset of data on disk
 *
 * @return: size of successful decoding (in bytes)
 */
int CodingLayer::decoding_mbr(int disk_id, char *buf, long long size,
			      long long offset)
{
	int retstat;
	struct timeval t1, t2, t3, t4;
	double duration;

	if (NCFS_DATA->disk_status[disk_id] == 0) {
		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t1, NULL);
		}

		retstat = cacheLayer->DiskRead(disk_id, buf, size, offset);

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t2, NULL);
		}

		return retstat;
	} else {
		char *temp_buf;
		char *buf_temp;

		int i;
		long long j;
		temp_buf = (char *)malloc(sizeof(char) * size);
		memset(temp_buf, 0, size);

		memset(buf, 0, size);

		buf_temp = (char *)malloc(sizeof(char) * size);
		memset(buf_temp, 0, size);

		int *intbuf = (int *)buf;
		int *intbuf_temp = (int *)buf_temp;

		int *erased;	//record the surviving or fail of each disk

		int *decoding_matrix;
		int *dm_ids;	//record the ids of the selected disks

		int mbr_segment_size;
		int mbr_segment_id;
		int mbr_block_id;
		long long block_no;
		int dup_disk_id, dup_block_no;
		int temp_mbr_block_id, temp_disk_id, temp_dup_disk_id,
		    temp_block_no;
		int flag_dup;
		int disk_failed_no, disk_another_failed_id;
		long long offset_read;
		int temp;

		//check the number of failed disks
		disk_failed_no = 0;
		disk_another_failed_id = -1;
		for (i = 0; i < NCFS_DATA->disk_total_num; i++) {
			if (NCFS_DATA->disk_status[i] == 1) {
				(disk_failed_no)++;
				if (i != disk_id) {
					disk_another_failed_id = i;
				}
			}
		}

		mbr_segment_size = NCFS_DATA->mbr_segment_size;
		block_no = (long long)(offset / NCFS_DATA->chunk_size);
		mbr_segment_id = (int)(block_no / mbr_segment_size);

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t1, NULL);
		}

		flag_dup = 0;
		mbr_block_id =
		    mbr_find_block_id(disk_id, block_no, mbr_segment_size);
		if (mbr_block_id == -1) {
			flag_dup = 1;
			mbr_block_id =
			    mbr_find_dup_block_id(disk_id, block_no,
						  mbr_segment_size);
		}

		if (flag_dup == 0) {
			dup_disk_id =
			    mbr_get_dup_disk_id(mbr_block_id, mbr_segment_size);
			dup_block_no =
			    mbr_get_dup_block_no(disk_id, mbr_block_id,
						 mbr_segment_size);
			dup_block_no =
			    dup_block_no + mbr_segment_id * mbr_segment_size;
		} else {	//if flag_dup = 1
			dup_disk_id =
			    mbr_get_disk_id(mbr_block_id, mbr_segment_size);
			dup_block_no =
			    mbr_get_block_no(dup_disk_id, mbr_block_id,
					     mbr_segment_size);
			dup_block_no =
			    dup_block_no + mbr_segment_id * mbr_segment_size;
		}

		if (NCFS_DATA->run_experiment == 1) {
			gettimeofday(&t2, NULL);

			duration = (t2.tv_sec - t1.tv_sec) +
			    (t2.tv_usec - t1.tv_usec) / 1000000.0;
			NCFS_DATA->decoding_time += duration;
		}

		//Generalized E-MBR, you may have multiplie failures
		if (NCFS_DATA->disk_status[dup_disk_id] == 1) {
			//record the disk health information
			erased =
			    (int *)malloc(sizeof(int) *
					  (NCFS_DATA->mbr_m +
					   NCFS_DATA->mbr_c));

			//record the first m healthy blocks
			dm_ids =
			    (int *)malloc(sizeof(int) * (NCFS_DATA->mbr_m));

			//Use coded block to decode
			for (i = 0;
			     i < ((NCFS_DATA->mbr_m) + (NCFS_DATA->mbr_c));
			     i++) {

				temp_mbr_block_id = i;
				temp_disk_id =
				    mbr_get_disk_id(temp_mbr_block_id,
						    mbr_segment_size);
				temp_dup_disk_id =
				    mbr_get_dup_disk_id(temp_mbr_block_id,
							mbr_segment_size);

				//the block doesn't exist
				if ((NCFS_DATA->disk_status[temp_disk_id] == 1)
				    && (NCFS_DATA->
					disk_status[temp_dup_disk_id] == 1)) {
					erased[i] = 1;
				} else {
					erased[i] = 0;
				}
			}

			//allocate k*k space for decoding matrix
			decoding_matrix =
			    (int *)malloc(sizeof(int) * (NCFS_DATA->mbr_m) *
					  (NCFS_DATA->mbr_m));

			if (jerasure_make_decoding_matrix
			    ((NCFS_DATA->mbr_m), (NCFS_DATA->mbr_c),
			     field_power, NCFS_DATA->generator_matrix, erased,
			     decoding_matrix, dm_ids) < 0) {
				free(erased);
				free(dm_ids);
				free(decoding_matrix);
				return -1;
			}
			//travel the selected surviving blocks
			for (i = 0; i < (NCFS_DATA->mbr_m); i++) {

				memset(temp_buf, 0, size);

				//decoding efficient for this surviving block
				int efficient =
				    decoding_matrix[NCFS_DATA->mbr_m *
						    mbr_block_id + i];

				//real block id
				temp_mbr_block_id = dm_ids[i];

				temp_disk_id =
				    mbr_get_disk_id(temp_mbr_block_id,
						    mbr_segment_size);
				temp_block_no =
				    mbr_get_block_no(temp_disk_id,
						     temp_mbr_block_id,
						     mbr_segment_size);

				if (NCFS_DATA->disk_status[temp_disk_id] == 1) {
					//original data block fails, use duplicated block
					temp = temp_disk_id;
					temp_disk_id =
					    mbr_get_dup_disk_id
					    (temp_mbr_block_id,
					     mbr_segment_size);
					temp_block_no =
					    mbr_get_dup_block_no(temp,
								 temp_mbr_block_id,
								 mbr_segment_size);
				}

				temp_block_no =
				    temp_block_no +
				    mbr_segment_id * mbr_segment_size;
				offset_read =
				    (long long)(temp_block_no *
						(NCFS_DATA->chunk_size));

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t2, NULL);
				}

				retstat =
				    cacheLayer->DiskRead(temp_disk_id, temp_buf,
							 size, offset_read);

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t3, NULL);
				}

				if (retstat < 0) {
					return -1;
				}

				for (j = 0; j < size; j++) {
					buf_temp[j] =
					    galois_single_multiply((unsigned
								    char)
								   temp_buf[j],
								   efficient,
								   field_power);
				}

				for (j = 0;
				     j <
				     (long long)(size * sizeof(char) /
						 sizeof(int)); ++j) {
					intbuf[j] = intbuf[j] ^ intbuf_temp[j];
				}

				if (NCFS_DATA->run_experiment == 1) {
					gettimeofday(&t4, NULL);

					duration =
					    (t2.tv_sec - t1.tv_sec) +
					    (t2.tv_usec -
					     t1.tv_usec) / 1000000.0;
					NCFS_DATA->decoding_time += duration;

					duration =
					    (t4.tv_sec - t3.tv_sec) +
					    (t4.tv_usec -
					     t3.tv_usec) / 1000000.0;
					NCFS_DATA->decoding_time += duration;
				}

			}

		} else {
			//Use duplicated block to decode

			offset_read =
			    (long long)(dup_block_no *
					(NCFS_DATA->chunk_size));

			if (NCFS_DATA->run_experiment == 1) {
				gettimeofday(&t1, NULL);
			}

			retstat =
			    cacheLayer->DiskRead(dup_disk_id, buf, size,
						 offset_read);

			if (NCFS_DATA->run_experiment == 1) {
				gettimeofday(&t2, NULL);
			}

			return retstat;
		}

		free(erased);
		free(dm_ids);
		free(decoding_matrix);
		free(temp_buf);
		return size;
	}

	AbnormalError();
	return -1;
}

/*
 * decoding_rs: Reed-Solomon decode
 *
 * @param disk_id: disk id               
 * @param buf: data buffer for reading data
 * @param size: size of the data buffer                  
 * @param offset: offset of data on disk
 *
 * @return: size of successful decoding (in bytes)
 */
int CodingLayer::decoding_rs(int disk_id, char *buf, long long size,
			     long long offset)
{
	int retstat;

	//a healthy data disk
	if (NCFS_DATA->disk_status[disk_id] == 0) {

		retstat = cacheLayer->DiskRead(disk_id, buf, size, offset);

		return retstat;
	} else {
		
		char *temp_buf;
		char *buf_temp;
		int i;
		long long j;

		int *erased;	//record the surviving or fail of each disk

		int *decoding_matrix;
		int *dm_ids;	//record the ids of the selected disks

		int disk_total_num;
		int data_disk_num;

		disk_total_num = NCFS_DATA->disk_total_num;
		data_disk_num = NCFS_DATA->data_disk_num;

		int block_size;
		int block_no;

		int blockoffset;
		int diskblocklocation;	// indicates the block location

		block_size = NCFS_DATA->chunk_size;
		block_no = offset / block_size;
		blockoffset = offset - block_no * block_size;

		temp_buf = (char *)malloc(sizeof(char) * size);
		memset(temp_buf, 0, size);

		buf_temp = (char *)malloc(sizeof(char) * size);
		memset(buf_temp, 0, size);

		memset(buf, 0, size);

		int *intbuf_temp = (int *)buf_temp;
		int *intbuf = (int *)buf;

		//record the disk health information
		erased = (int *)malloc(sizeof(int) * (disk_total_num));

		//record the first k healthy disks
		dm_ids = (int *)malloc(sizeof(int) * (data_disk_num));

		//find all the failed disks
		for (i = 0; i < disk_total_num; i++) {
			if (NCFS_DATA->disk_status[i] == 1) {

				erased[i] = 1;
			} else {
				erased[i] = 0;
			}
		}

		//allocate k*k space for decoding matrix
		decoding_matrix =
		    (int *)malloc(sizeof(int) * data_disk_num * data_disk_num);

		if (jerasure_make_decoding_matrix
		    (data_disk_num, (disk_total_num - data_disk_num),
		     field_power, NCFS_DATA->generator_matrix, erased,
		     decoding_matrix, dm_ids) < 0) {
			free(erased);
			free(dm_ids);
			free(decoding_matrix);
			return -1;
		}

		//travel the selected surviving disks
		for (i = 0; i < data_disk_num; i++) {

			diskblocklocation = block_no * block_size + blockoffset;

			int efficient =
			    decoding_matrix[data_disk_num * disk_id + i];

			//processed data block
			retstat =
			    cacheLayer->DiskRead(dm_ids[i], temp_buf, size,
						 diskblocklocation);

			for (j = 0; j < size; j++) {
				buf_temp[j] =
				    galois_single_multiply((unsigned char)
							   temp_buf[j],
							   efficient,
							   field_power);
			}

			for (j = 0;
			     j < (long long)(size * sizeof(char) / sizeof(int));
			     ++j) {
				intbuf[j] = intbuf[j] ^ intbuf_temp[j];
			}

			memset(temp_buf, 0, size);
		}

		free(erased);
		free(dm_ids);
		free(decoding_matrix);
		free(temp_buf);
		free(buf_temp);

		return size;
	}

	AbnormalError();
	return -1;
}

/*************************************************************************
 * Public functions
 *************************************************************************/

/*
 * Constructor - Initialize Coding_storage layer
 */
CodingLayer::CodingLayer()
{

	int coding_type;

	coding_type = NCFS_DATA->disk_raid_type;

	if (coding_type == 6) {	//raid 6
		gf_gen_tables(field_power);
		//generate GF tables for raid6
	}

	//Add by Dongsheng Wei on Jan. 16, 2014 begin.
	if(coding_type == 5000){
		mdr_I_encoding_matrixB = mdr_I_encoding_matrix(NCFS_DATA->data_disk_num);
		strip_size = (int)pow(2, NCFS_DATA->data_disk_num);
		mdr_I_one_dpDisk_fail_bool_m = false;
		mdr_I_one_dpDisk_fail_bool_v = false;
	}
	//Add by Dongsheng Wei on Jan. 16, 2014 end.
}

//Add by Dongsheng Wei on Jan. 16, 2014 begin.
CodingLayer :: ~CodingLayer(){
	if(NCFS_DATA->disk_raid_type == 5000)
		delete []mdr_I_encoding_matrixB;
}
//Add by Dongsheng Wei on Jan. 16, 2014 end.

/*
 * encode: Write data to disk and return written block info
 *
 * @param buf: data buffer to be written to disk
 * @param size: size of the data buffer 
 *
 * @return: assigned data block: disk id and block number.
 */
struct data_block_info CodingLayer::encode(const char *buf, int size)
{
	struct data_block_info block_written;

	switch (NCFS_DATA->disk_raid_type) {
	case 0:
		block_written = encoding_raid0(buf, size);
		break;
	case 1:
		block_written = encoding_raid1(buf, size);
		break;
	case 4:
		block_written = encoding_raid4(buf, size);
		break;
	case 5:
		block_written = encoding_raid5(buf, size);
		break;
	case 6:
		block_written = encoding_raid6(buf, size);
		break;
	case 100:
		block_written = encoding_jbod(buf, size);
		break;
	case 1000:
		block_written = encoding_mbr(buf, size);
		break;
	case 3000:
		block_written = encoding_rs(buf, size);
		break;
	//Add by Dongsheng Wei on Jan. 16, 2014 begin.
	case 5000:
		block_written = encoding_mdr_I(buf, size);
		break;
	case 5001:
		block_written = encoding_raid5_noRotate(buf, size);
		break;
	case 6001:
		block_written = encoding_raid6_noRotate(buf, size);
		break;
	//Add by Dongsheng Wei on Jan. 16, 2014 end.
	default:
		return encoding_default(buf, size);
		break;
	}

	if (NCFS_DATA->no_gui == 0) {
		diskusageLayer->Update(block_written.disk_id,
				       block_written.block_no, 1);
	}

	return block_written;
}

/*
 * decode: Select decoding function and read data
 *
 * @param disk_id: disk id
 * @param buf: data buffer for reading data
 * @param size: size of the data buffer
 * @param offset: offset of data on disk
 * 
 * @return: size of successful read (in bytes)
 */
int CodingLayer::decode(int disk_id, char *buf, long long size,
			long long offset)
{
	switch (NCFS_DATA->disk_raid_type) {
	case 0:
		return decoding_raid0(disk_id, buf, size, offset);
		break;
	case 1:
		return decoding_raid1(disk_id, buf, size, offset);
		break;
	case 4:
		return decoding_raid4(disk_id, buf, size, offset);
		break;
	case 5:
		return decoding_raid5(disk_id, buf, size, offset);
		break;
	case 6:
		return decoding_raid6(disk_id, buf, size, offset);
		break;
	case 100:
		return decoding_jbod(disk_id, buf, size, offset);
		break;
	case 1000:
		return decoding_mbr(disk_id, buf, size, offset);
		break;
	case 3000:
		return decoding_rs(disk_id, buf, size, offset);
		break;
	//Add by Dongsheng Wei on Jan. 16, 2014 begin.
	case 5000:
		return decoding_mdr_I(disk_id, buf, size, offset);
		break;
	case 5001:
		return decoding_raid5_noRotate(disk_id, buf, size, offset);
		break;
	case 6001:
		return decoding_raid6_noRotate(disk_id, buf, size, offset);
		break;
	//Add by Dongsheng Wei on Jan. 16, 2014 end;
	default:
		return decoding_default(disk_id, buf, size, offset);
		break;
	}
	AbnormalError();
	return -1;
}
