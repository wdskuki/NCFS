#ifndef __CODING_HH__
#define __CODING_HH__

#include "../filesystem/filesystem_common.hh"

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

 public:
	 CodingLayer();
	struct data_block_info encode(const char *buf, int size);
	int decode(int disk_id, char *buf, long long size, long long offset);
};

#endif
