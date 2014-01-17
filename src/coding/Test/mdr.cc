#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <vector>
#include <iostream>
using namespace std;
int k;
int strip_size;
long long* matrixB; 

long long * mdr_I_iterative_construct_encoding_matrixB(long long *matrix, 
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

void mdr_print_matrix(long long* matrix, int row, int col){
	for(int i = 0; i < row; i++){
		for(int j = 0; j < col; j++)
			printf("%lld\t",matrix[i*col+j]);
		printf("\n");
	}
}

long long* mdr_I_encoding_matrix(int k){ 
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

vector<int> mdr_I_find_parity_blocks_id(int disk_id, int block_no){
	int strip_size = (int)pow(2, k);
	vector<int> ivec;

	int row = strip_size;
	int col = k+1;

	int t = 1 << (strip_size - block_no -1);
	for(int i = 0; i < row; i++){
		if((matrixB[i*col+disk_id]&t) != 0){
			ivec.push_back(i);
		}
	}
	return ivec;
}

void print_ivec(vector<int>& ivec){
	int size = ivec.size();
	for(int i = 0; i < size; i++){
		cout<<(ivec.at(i)+1)<<" ";
	}
	cout<<endl;
}
int main(int argc, char const *argv[])
{
	vector<int> ivec;
	k = 3;
	strip_size = (int)pow(2, k);

	matrixB = mdr_I_encoding_matrix(k);
	
	int row = (int)pow(2, k);
	int col = k+1;
	mdr_print_matrix(matrixB, row, col);

	cout<<"------"<<endl;
	for(int i = 0; i < k; i++){
		for(int j = 0; j < strip_size; j++){
			ivec = mdr_I_find_parity_blocks_id(i,j);
			printf("[%d, %d]: ", i+1, j+1);
			print_ivec(ivec);
		}
	}
	

	delete []matrixB;
	return 0;
}