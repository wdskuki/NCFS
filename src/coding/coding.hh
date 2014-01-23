#ifndef __CODING_HH__
#define __CODING_HH__

#include "../filesystem/filesystem_common.hh"
#include <vector>
#include <map>
using namespace std;
class CodingLayer {

 private:
	//GF operations
	unsigned short *gflog, *gfilog;
	static const unsigned int prim_poly_8 = 285;	//0x11d
	//primitive polynomial = x^8 + x^4 + x^3 + x^2 + 1
	static const int field_power = 8;
	//field size = 2^8

	int gf_gen_tables(int s);
	unsigned short gf_mul(int a, int b, int s);
	unsigned short gf_div(int a, int b, int s);
	int gf_get_coefficient(int value, int s);

	//MBR operations
	int mbr_find_block_id(int disk_id, int block_no, int mbr_segment_size);
	int mbr_find_dup_block_id(int disk_id, int block_no,
				  int mbr_segment_size);
	int mbr_get_disk_id(int mbr_block_id, int mbr_segment_size);
	int mbr_get_block_no(int disk_id, int mbr_block_id,
			     int mbr_segment_size);
	int mbr_get_dup_disk_id(int mbr_block_id, int mbr_segment_size);
	int mbr_get_dup_block_no(int disk_id, int mbr_block_id,
				 int mbr_segment_size);

	//MDR_I operation
	//Add by Dongsheng Wei on Jan. 17, 2014 begin.
	long long* mdr_I_encoding_matrixB;
	int strip_size;
	long long* mdr_I_iterative_construct_encoding_matrixB(long long *matrix, int k);
	long long* mdr_I_encoding_matrix(int k);
	void mdr_print_matrix(long long* matrix, int row, int col);	
	vector<int> mdr_I_find_q_blocks_id(int disk_id, int block_no);
	vector<vector<int> > mdr_I_repair_qDisk_blocks_id(int block_no);
	vector<int> mdr_I_repair_dpDisk_stripeIndexs_internal(int diskID, int val_k);
	vector<int> mdr_I_repair_dpDisk_stripeIndexs(int diskID, int val_k);
	bool mdr_I_repair_if_blk_in_buf(int disk_id, int stripe_blk_offset, bool ** isInbuf,
									 vector<int>& mdr_I_one_dpDisk_fail_stripeIndex);

	int mdr_I_repair_chg_blkIndexOffset_in_buf(int disk_id, int stripe_blk_offset, vector<int>& stripeIndexs);
	bool mdr_I_one_dpDisk_fail_bool_m;
	bool mdr_I_one_dpDisk_fail_bool_v;
	map<int, vector<vector<int> > > mdr_I_one_dpDisk_fail_nonStripeIndex;
	vector<int> mdr_I_one_dpDisk_fail_stripeIndex;
	map<int, vector<vector<int> > > mdr_I_repair_dpDisk_nonstripeIndexs_blocks_no(int fail_disk_id, 
														  vector<int>& stripeIndexs);

	void print_ivec(vector<int>& ivec);
	void print_iivec(vector<vector<int> >& iivec);
	void print_ivmap(map<int, vector<vector<int> > >& ivmap, vector<int>& stripeIndexs);
	//Add by Dongsheng Wei on Jan. 17, 2014 end.	
	
	//encoding
	struct data_block_info encoding_default(const char *buf, int size);
	struct data_block_info encoding_jbod(const char *buf, int size);
	struct data_block_info encoding_raid0(const char *buf, int size);
	struct data_block_info encoding_raid1(const char *buf, int size);
	struct data_block_info encoding_raid4(const char *buf, int size);
	struct data_block_info encoding_raid5(const char *buf, int size);
	struct data_block_info encoding_raid6(const char *buf, int size);
	struct data_block_info encoding_mbr(const char *buf, int size);	//type 1000; exact MBR
	struct data_block_info encoding_rs(const char *buf, int size);	//type 3000; Reed-Solomon
	//Add by Dongsheng Wei on Jan. 16, 2014 begin.	
	struct data_block_info encoding_mdr_I(const char *buf, int size); //type 5000; MDR I
	struct data_block_info encoding_raid5_noRotate(const char *buf, int size); //type 5001; raid5(no rotate)
	struct data_block_info encoding_raid6_noRotate(const char *buf, int size); //type 6001; raid6(no rotate)
	//Add by Dongsheng Wei on Jan. 16, 2014 end.
	
	//decoding
	int decoding_default(int disk_id, char *buf, long long size,
			     long long offset);
	int decoding_jbod(int disk_id, char *buf, long long size,
			  long long offset);
	int decoding_raid0(int disk_id, char *buf, long long size,
			   long long offset);
	int decoding_raid1(int disk_id, char *buf, long long size,
			   long long offset);
	int decoding_raid4(int disk_id, char *buf, long long size,
			   long long offset);
	int decoding_raid5(int disk_id, char *buf, long long size,
			   long long offset);
	int decoding_raid6(int disk_id, char *buf, long long size,
			   long long offset);
	int decoding_mbr(int disk_id, char *buf, long long size,
			 long long offset);
	int decoding_rs(int disk_id, char *buf, long long size,
			long long offset);
	//Add by Dongsheng Wei on Jan. 16, 2014 begin.
	int decoding_mdr_I(int disk_id, char *buf, long long size,
				long long offset);
	int decoding_raid5_noRotate(int disk_id, char *buf, long long size,
				long long offset);
	int decoding_raid6_noRotate(int disk_id, char *buf, long long size,
				long long offset);
	//Add by Dongsheng Wei on Jan. 16, 2014 end.

 public:
	 CodingLayer();
	 ~CodingLayer();
	struct data_block_info encode(const char *buf, int size);
	int decode(int disk_id, char *buf, long long size, long long offset);

	int mdr_I_recover_oneStripeGroup(int disk_id, char *buf, long long size,
								long long offset, char*** pread_stripes);
	
	int mdr_I_get_strip_size(){return strip_size;}
};

#endif
