/*
 * netcodec.hh
 */

#ifndef __netcodec_hh__
#define __netcodec_hh__

#include <iostream>
#include <stdint.h>
#include <vector>
#include "GaloisField.h"
//#include "../GaloisField/GaloisField.h"

// CLICK_DECLS

using namespace std;
using namespace galois;

class CodeBlock {
private:
	// global variables
	static bool is_data_;			// is data to be processed
	static int symb_size_;			// number of coded symbols
	static GaloisField gf_;			// galois field

	// data members
	int first_;						// index of the first non-zero coefficient
	int k_;							// number of coefficients
	uint8_t* coeff_;				// list of coefficients	
	uint8_t* symb_;					// an array of coded symbols

public:
	CodeBlock(int k);
	CodeBlock(int k, uint8_t* coeff, uint8_t* symb = NULL, int len = 0);
	~CodeBlock();
	void add(CodeBlock* cb, uint8_t h);
	void normalize();
	void print();
	CodeBlock *  copy();


	// inline function
	inline int first() { return first_; }
	inline uint8_t coeff(int i) { return coeff_[i]; }
	inline uint8_t* symb() { return symb_; }
	inline static int symb_size() { return symb_size_; }
	inline static void set_symb_size(int symb_size) { symb_size_ = symb_size; }
};


class CodeMatrix {
private:
	// data members
	int k_;					  // matrix size, i.e., k_ * k_
	int rank_;				// current rank of the matrix
  CodeBlock** cb_;  //k_  code blocks 
public:
	CodeMatrix(int k); 
	bool addEncodedBlock(CodeBlock* block);
	bool storeEncodedBlock(CodeBlock* block);
	void reset();
	void print();

	// inline function
	inline int rank() { return rank_; }
	inline CodeBlock* cb(int i) { return cb_[i]; }
	inline uint8_t coeff(int i, int j) { return cb_[i]->coeff(j); }
}; 


class NetCodec {
private:
  int k_;								// matrix size, i.e., k_ * k_; = MBR_original_block_num
  int e_;                //=MBR_encoded_block_num
	vector<CodeBlock*> dec_list_;		// list of decoded packets
	CodeMatrix* orig_matrix_;			// original matrix which stores all the k_ original blocks
	CodeMatrix* dec_matrix_;			// decoding matrix which stores no more than k_ encoded blocks with normalization.
	CodeMatrix* enc_matrix_;      // encoding matrix which stores all the encoded blocks
	
	

public:
	NetCodec(int k,int e); 
	void addNativePacket(uint8_t* data, int len); 
	bool addCodedPacket(uint8_t* coeff, uint8_t* data, int len); 
	CodeBlock* encode(); 
	void store_encode(CodeBlock* block);
	void decode(CodeBlock* block); 
	void getNativePacket(uint8_t* ret_data); 
	bool isDecodable(); 
	void reset(); 
	void print();
	

	// inline functions
	inline int k() { return k_; }
	inline int orig_matrix_rank() { return orig_matrix_->rank(); }
	inline int dec_matrix_rank() { return dec_matrix_->rank(); }
	inline int enc_matrix_rank() { return dec_matrix_->rank(); }
	inline CodeBlock* orig_matrix_block(int i){return orig_matrix_->cb(i);}
	inline CodeBlock* dec_matrix_block(int i){return dec_matrix_->cb(i);}
	inline CodeBlock* enc_matrix_block(int i){return enc_matrix_->cb(i);}
}; 

// CLICK_ENDDECLS

#endif
