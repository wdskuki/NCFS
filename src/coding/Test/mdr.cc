#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <vector>
#include <set>
#include <map>
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

vector<int> mdr_I_find_q_blocks_id(int disk_id, int block_no){
	int strip_size = (int)pow(2, k);
	vector<int> ivec;

	int row = strip_size;
	int col = k+1;

	int t = 1 << (strip_size - block_no -1);

	
	for(int i = 0; i < row; i++){
		//data block affect q disk
		if((matrixB[i*col+disk_id]&t) != 0){
			ivec.push_back(i);
		}
		
		//parity block (disk_id) affects q disk
		if((matrixB[i*col+k]&t) != 0){
			ivec.push_back(i);
		}
	}

	return ivec;
}

vector<vector<int> > mdr_I_repair_qDisk_blocks_id(int block_no){
	vector<vector<int> > iivec;
	int row = strip_size;
	int col = k+1;
	for(int i = 0; i < col; i++){
		vector<int> ivec;
		for(int j = 0; j < strip_size; j++){
			if((matrixB[block_no*col+i]&(1<<(strip_size-j-1))) != 0){
				ivec.push_back(j);
			}
		}
		iivec.push_back(ivec);
	}
	return iivec;
}


// //it is wrong!
// vector<int> mdr_I_repair_dpDisk_stripeIndexs_less_k(){
// 	int r = strip_size / 2;
// 	vector<int> ivec;
// 	for(int i = 0; i < r; i++){
// 		ivec.push_back(i*2);
// 	}
// 	return ivec;
// }

// vector<int> mdr_I_repair_dpDisk_stripeIndexs_equal_k(){
// 	int r = strip_size / 2;
// 	vector<int> ivec;
// 	for(int i = 0; i < r; i++){
// 		//cout<<"mdr_I_repair_dqDisk_stripeIndexs_equal_k: "<<i<<endl;
// 		ivec.push_back(i);
// 	}
// 	return ivec;
// }

// vector<int> mdr_I_repair_dpDisk_stripeIndexs_more_k(){
// 	int r = strip_size / 2;
// 	vector<int> ivec;
// 	for(int i = 0; i < r; i++){
// 		ivec.push_back(i+r);
// 	}
// 	return ivec;	
// }

// vector<int> mdr_I_repair_dpDisk_stripeIndexs(int diskID){
// 	vector<int> ivec;
// 	if(diskID > k){
// 		cout<<"error in mdr_I_repair_dpDisk_stripeIndexs()"<<endl;
// 		return ivec;
// 	}
// 	if(k == 1){
// 		if(diskID == 0)
// 			return mdr_I_repair_dpDisk_stripeIndexs_equal_k();
// 		else if(diskID == 1)
// 			return mdr_I_repair_dpDisk_stripeIndexs_more_k();
// 	}else if(k > 1){
// 		if(diskID >=0 && diskID < k - 1)
// 			return mdr_I_repair_dpDisk_stripeIndexs_less_k();
// 		else if(diskID == k - 1)
// 			return mdr_I_repair_dpDisk_stripeIndexs_equal_k();
// 		else if(diskID == k)
// 			return mdr_I_repair_dpDisk_stripeIndexs_more_k();
// 	}	
// }

vector<int> mdr_I_repair_dpDisk_stripeIndexs_internal(int diskID, int val_k){
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
vector<int> mdr_I_repair_dpDisk_stripeIndexs(int diskID, int val_k){
	vector<int> ivec = mdr_I_repair_dpDisk_stripeIndexs_internal(diskID+1, val_k);
	int ivec_size = ivec.size();

	vector<int> ivec2;
	for(int i = 0; i < ivec_size; i++){
		ivec2.push_back(ivec[i]-1);
	}
	return ivec2;
}



map<int, vector<vector<int> > > mdr_I_repair_dpDisk_nonstripeIndexs_blocks_no(int fail_disk_id, 
														  vector<int>& stripeIndexs){

	map<int, vector<vector<int> > > ivmap;
	int row = strip_size;
	int col = k+1;

	set<int> iset;
	iset.insert(stripeIndexs.begin(), stripeIndexs.end());

	int stripeIndexs_size = stripeIndexs.size();
	for(int i = 0; i < stripeIndexs_size; i++){
		vector<vector<int> > iivec;
		int fail_disk_block_no = -1;
		for(int j = 0; j < col; j++){
			int val = matrixB[stripeIndexs[i]*col+j];
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



void print_ivec(vector<int>& ivec){
	int size = ivec.size();
	for(int i = 0; i < size; i++){
		cout<<(ivec.at(i))<<" ";
	}
	cout<<endl;
}

void print_iivec(vector<vector<int> >& iivec){
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

void print_ivmap(map<int, vector<vector<int> > >& ivmap, vector<int>& stripeIndexs){
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

int main(int argc, char const *argv[])
{
	vector<int> ivec;
	vector<vector<int> > iivec;
	map<int, vector<vector<int> > > ivmap;

	k = 3;
	strip_size = (int)pow(2, k);

	matrixB = mdr_I_encoding_matrix(k);
	int fail_disk_id = 2;

	int row = (int)pow(2, k);
	int col = k+1;
	mdr_print_matrix(matrixB, row, col);

	cout<<"------"<<endl;
	for(int i = 0; i < k; i++){
		for(int j = 0; j < strip_size; j++){
			ivec = mdr_I_find_q_blocks_id(i,j);
			printf("[%d, %d]: ", i, j);
			print_ivec(ivec);
		}
	}

	cout<<"------"<<endl;
	for(int i = 0; i < strip_size; i++){
		vector<vector<int> > iivec = mdr_I_repair_qDisk_blocks_id(i);
		print_iivec(iivec);
	}

	cout<<"------"<<endl;
	ivec = mdr_I_repair_dpDisk_stripeIndexs(fail_disk_id, k);
	print_ivec(ivec);
	ivmap = mdr_I_repair_dpDisk_nonstripeIndexs_blocks_no(fail_disk_id, ivec);
	print_ivmap(ivmap, ivec);

	// for(int i = 0; i <= k; i++){
	// 	ivec = mdr_I_repair_dqDisk_stripeIndexs(i);
	// 	print_ivec(ivec);
	// }

	
	delete []matrixB;
	return 0;
}