#include <math.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
using namespace std;

void print_R(int *R, int k){
	int col = (int)pow(2, k-1);
	for(int i = 0; i < col; i++)
		cout<<R[i]<<" ";
	cout<<endl;
}
void print_2d_matrix(int **matrix, int row, int col){
	for(int i = 0; i < row; i++){
		for(int j = 0; j < col; j++)
			cout<<matrix[i][j]<<" ";
		cout<<endl;
	}
}
void print_array(int *array, int num){
	for(int i = 0; i < num; i++)
		cout<<array[i]<<" ";
	cout<<endl;
}

void clear_row(int **stp_gp, int *p, int k, int failID){
	int row = k;
	int col = (int)pow(2, k);

	if( failID <= 0 && failID >= k+2){
		cout<<"error: failID in clear_row()"<<endl;
		return;
	}
	if(failID <= k){
		for(int i = 0; i < col; i++)
			stp_gp[failID-1][i] = -1;
	}else if(failID == k+1){
		for(int i = 0; i < col; i++)
			p[i] = -1;
	}
}

int ***mdr_new_3d_matrix(int k)
{	
 	int row = (int)pow(2, k);
 	int col = row;
 	int ***matrix;

 	matrix = new int **[k];
 	for(int i = 0; i < k; i++){
 		matrix[i] = new int *[row];
 		for(int j = 0; j < row; j++)
 			matrix[i][j] = new int [col];
 	}

 	return matrix;
}

void mdr_delete_3d_matrix(int ***matrix, int k)
{
 	int row = (int)pow(2, k);
 	int col = row;

 	for(int i = 0; i < k; i++){
 		for(int j = 0; j < row; j++){
 			delete []matrix[i][j];
 		}
 		delete []matrix[i];
 	}
 	delete []matrix;
 	matrix = NULL;
}


int ***mdr_new_3d_matrix_B(int k)
{	
 	int row = (int)pow(2, k);
 	int col = row;
 	int ***matrix;

 	matrix = new int **[k+1];
 	for(int i = 0; i < k+1; i++){
 		matrix[i] = new int *[row];
 		for(int j = 0; j < row; j++)
 			matrix[i][j] = new int [col];
 	}

 	return matrix;
}

void mdr_delete_3d_matrix_B(int ***matrix, int k)
{
 	int row = (int)pow(2, k);
 	int col = row;

 	for(int i = 0; i < k+1; i++){
 		for(int j = 0; j < row; j++){
 			delete []matrix[i][j];
 		}
 		delete []matrix[i];
 	}
 	delete []matrix;
 	matrix = NULL;
}


int ***mdr_seed_matrix(){
	int k = 1;
	int ***seed_matrix = mdr_new_3d_matrix(k);
	seed_matrix[0][0][0] = 0;
	seed_matrix[0][0][1] = 1;
	seed_matrix[0][1][0] = 1;
	seed_matrix[0][1][1] = 0;
	return seed_matrix;	
}

int*** mdr_iterative_construct(int ***src, int k){
	int ***des = mdr_new_3d_matrix(k+1);
	int row = (int)pow(2, k);
	int col = row;

	for(int t = 0; t < k; t++){
		for(int i = 0; i < row; i++)
			for(int j = 0; j < col; j++)
				des[t][i][j] = src[t][i][j];

		for(int i = 0; i < row; i++)
			for(int j = 0; j < col; j++)
				des[t][i][j+col] = 0;

		for(int i = 0; i < row; i++)
			for(int j = 0; j < col; j++){
				if(i == j)
					des[t][i+row][j] = 1;
				else
					des[t][i+row][j] = 0;
			}

		for(int i = 0; i < row; i++)
			for(int j = 0; j < col; j++)
				des[t][i+row][j+col] = src[t][i][j];

	}

	for(int i = 0; i < row; i++)
		for(int j = 0; j < col; j++)
			des[k][i][j] = 0;

	for(int i = 0; i < row; i++)
		for(int j = 0; j < col; j++){
			if(i == j)
				des[k][i][j+col] = 1;
			else
				des[k][i][j+col] = 0;
		}

	for(int i = 0; i < row; i++)
		for(int j = 0; j < col; j++){
			if(i == j)
				des[k][i+row][j] = 1;
			else
				des[k][i+row][j] = 0;
		}

	for(int i = 0; i < row; i++)
		for(int j = 0; j < col; j++)
			des[k][i][j] = 0;

	mdr_delete_3d_matrix(src, k);
	return des;
}

int ***mdr_encoding_matrix(int k){
	if(k <= 0){
		cout<<"error: in mdr_encoding_matrix\n";
		return NULL;
	}
	if( k == 1)
		return mdr_seed_matrix();
	else{
		int tmp = 1;
		int ***encoding_matrix = mdr_seed_matrix();
		while(tmp != k){
			encoding_matrix = mdr_iterative_construct(encoding_matrix, tmp);
			tmp++;
		}
		return encoding_matrix;
	}
}
void mdr_print_matrix(int ***matrix, int k){
	int row = (int)pow(2, k);
	int col = row;
	for(int j = 0; j < row; j++){
		for(int i = 0; i < k; i++){
			for(int t = 0; t < col; t++){
				cout<<matrix[i][j][t]<<" ";
			}
			cout<<" ";
		}
		cout<<endl;
	}
}

void mdr_print_matrix_B(int ***matrix, int k){
	int row = (int)pow(2, k);
	int col = row;
	for(int j = 0; j < row; j++){
		for(int i = 0; i < k+1; i++){
			for(int t = 0; t < col; t++){
				cout<<matrix[i][j][t]<<" ";
			}
			cout<<" ";
		}
		cout<<endl;
	}
}

void xor_2d_matrix(int **src1, int **src2, int **des, int row, int col){
	if(src1 == NULL && src1 == NULL){
		des = NULL;
		return;
	}

	if(src1 == NULL){
		for(int i = 0; i < row; i++){
			for(int j = 0; j < col; j++){
				des[i][j] = src2[i][j];
			}
		}
		return;
	}

	if(src2 == NULL){
		for(int i = 0; i < row; i++){
			for(int j = 0; j < col; j++){
				des[i][j] = src1[i][j];
			}
		}
		return;
	}

	for(int i = 0; i < row; i++){
		for(int j = 0; j < col; j++){
			des[i][j] = src1[i][j] ^ src2[i][j];
		}
	}

}

int ***  mdr_matrixA_2_matrixB(int ***matrixA, int k){
	int row = (int)pow(2, k);
	int col = row;

	int ***matrixB = new int **[k+1];
	for(int i = 0; i < k+1; i++){
		matrixB[i] = new int *[row];
		for(int j = 0; j < row; j++){
			matrixB[i][j] = new int [row];
		}
	}

	int **temp_B = new int *[row];
	for(int i = 0; i < row; i++)
		temp_B[i] = new int [col];
	
	int t = row/2;
	for(int i = 0; i < row; i++){
		for(int j = 0; j < col; j++){
			if(i-t == j)
				temp_B[i][j] = 1;
			else
				temp_B[i][j] = 0;
		}
	}

	for(int i = 0; i < k; i++)
		xor_2d_matrix(temp_B, matrixA[i], matrixB[i], row, col);
	xor_2d_matrix(temp_B, NULL, matrixB[k], row, col);

	//delete
	for(int i = 0; i < row; i++)
		delete []temp_B[i];
	delete []temp_B;
	temp_B=NULL;

	//mdr_delete_3d_matrix(matrixA, k);

	return matrixB;

}

int** mdr_stripe_group_random_generate(int k){
	srand(time(NULL));
	int row = k;
	int col = (int)pow(2, k);
	int **stp_gp = new int *[row];
	for(int i = 0; i < row; i++)
		stp_gp[i] = new int [col];
	for(int i = 0; i < row; i++)
		for(int j = 0; j < col; j++)
			stp_gp[i][j] = rand()%256;
	return stp_gp;
}

int* mdr_generate_p(int **stp_gp, int k){
	int row = k;
	int col = (int)pow(2, k);
	int *p = new int [col];

	for(int j = 0; j < col; j++){
		p[j] = 0;
		for(int i = 0; i < row; i++){
			p[j] = p[j] ^ stp_gp[i][j];
		}
	}
	return p;
}

int* mdr_generate_q(int **stp_gp, int k, int ***encoding_matrix){
	int sg_row = k;
	int sg_col = (int)pow(2, k);
	int m_row = sg_col;
	int m_col = m_row;

	int *q = new int [sg_col];

	for(int j = 0; j < m_row; j++){
		q[j] = 0;
		for(int i = 0; i < k; i++){
			for(int t = 0; t < m_col; t++){
				if(encoding_matrix[i][j][t] == 1){
					q[j] = q[j] ^ stp_gp[i][t];
				}
			}
		}
	}
	return q;
}

int* mdr_generate_q_by_matrixB(int **stp_gp, int *p, int k, int ***matrixB){
	int sg_row = k;
	int sg_col = (int)pow(2, k);
	int m_row = sg_col;
	int m_col = m_row;

	int *q = new int [sg_col];

	for(int j = 0; j < m_row; j++){
		q[j] = 0;
		for(int i = 0; i < k+1; i++){
			for(int t = 0; t < m_col; t++){
				if(matrixB[i][j][t] == 1){
					if(i < k)
						q[j] = q[j] ^ stp_gp[i][t];
					else if(i == k)
						q[j] = q[j] ^ p[t];
				}
			}
		}
	}
	return q;
}


int* mdr_repair_stripeIndex_by_matrixB_less_k(int k){
	if(k <= 1)
		return NULL;
	else{
		int r = (int)pow(2, k-1);
		int *R = new int [r];
		for(int i = 0; i < r; i++)
			R[i] = 2*i+1;
		return R;
	}
}
int* mdr_repair_stripeIndex_by_matrixB_equal_k(int k){
	int r = (int)pow(2, k-1);
	int *R = new int[r];
	if(k == 1){
		R[0] = 1;
		return R;
	}
	for(int i = 0; i < r; i++){
		R[i] = i+1;
	}
	return R;
}

int* mdr_repair_stripeIndex_by_matrixB_large_k(int k){
	int r = (int)pow(2, k-1);
	int *R = new int[r];
	if(k == 1){
		R[0] = 2;
		return R;
	}
	for(int i = 0; i < r; i++)
		R[i] = i+1+r;
	return R;
}

//diskID: 1,2,3,..,k,k+1,k+2
int* mdr_repair_stripeIndex_by_matrixB(int k, int diskID){
	if(diskID > k+1){
		cout<<"error: diskID(large than 'k+1')"<<endl;
		return NULL;
	}

	if(k == 1){
		if(diskID == 1)
			return mdr_repair_stripeIndex_by_matrixB_equal_k(k);
		else if(diskID == 2)
			return mdr_repair_stripeIndex_by_matrixB_large_k(k);
	}else if(k > 1){
		if(diskID >=1 && diskID < k)
			return mdr_repair_stripeIndex_by_matrixB_less_k(k);
		else if(diskID == k)
			return mdr_repair_stripeIndex_by_matrixB_equal_k(k);
		else if(diskID == k+1)
			return mdr_repair_stripeIndex_by_matrixB_large_k(k);
	}
}

void mdr_repair_one_dataDisk(int **stp_gp, int *p, int *q, int failID, int *R, int k, int ***matrixB){
	int sg_row = k;
	int sg_col = (int)pow(2, k);

	int p_col = sg_col;
	int q_col = sg_col;

	int R_col = (int)pow(2, k-1);

	int m_row = sg_col;
	int m_col = sg_col; 

	int *repairArray = new int [p_col];
	for(int i = 0; i < p_col; i++)
		repairArray[i] = 0;

	if(failID>= 1 && failID <= k){
		for(int i = 0; i < R_col; i++){
			for(int j = 0; j < k; j++){
				if( j != failID-1)
					repairArray[R[i]-1] ^= stp_gp[j][R[i]-1];
 			}
 			repairArray[R[i]-1] ^= p[R[i]-1];
		}
		
	}

	cout<<"DEBUG\n";
	print_array(repairArray, p_col);
}


// int* mdr_repair_p(int **stp_gp, int k){
// 	return mdr_generate_p(stp_gp, k);
// }

// int* mdr_repair_q(int **stp_gp, int k, int ***encoding_matrix){
// 	return mdr_generate_q(stp_gp, k, encoding_matrix);
// }

// int* mdr_repair_data(int **stp_gp, int *q,
// 					 int k, int fail_disk_id, 
// 					 int ***encoding_matrix){
	
// 	int sg_row = k;
// 	int sg_col = (int)pow(2, k);
// 	int m_row = sg_col;
// 	int m_col = m_row;

// 	int *new_comer = new int [sg_col];

// 	for(int j = 0; j < m_row; j++) new_comer[j] = 0;

// 	for(int j = 0; j < m_row; j++){
// 		for(int i = 0; i < k; i++){
// 			for(int t = 0; t < m_col; t++){
// 				//TODO:
// 			}
// 		}
// 	}
// }


int main(int argc, char const *argv[])
{
	
	if(argc != 2){
		cout<<"error: (please input 'k')\n";
		return -1;
	}

	int k = atoi(argv[1]);
	int stp_size = (int)pow(2,k);

	int fail_disk_id = 1;

	//generate encoding_matrix_A
	int ***encoding_matrix = mdr_encoding_matrix(k);
	cout<<"\nencoding_matrix_A:\n";
	mdr_print_matrix(encoding_matrix, k);

	int ***encoding_matrix_B = mdr_matrixA_2_matrixB(encoding_matrix, k);
	cout<<"\nencoding_matrix_B:\n";
	mdr_print_matrix_B(encoding_matrix_B, k);	

	//generate random stripe_group
	int **stp_gp = mdr_stripe_group_random_generate(k);
	cout<<"\nrandom_stripe_group:\n";
	print_2d_matrix(stp_gp, k, stp_size);

	//generate P array
	int *p = mdr_generate_p(stp_gp, k);
	cout<<"\nP:\n";
	print_array(p, stp_size);

	//generate Q array
	int *q = mdr_generate_q(stp_gp, k, encoding_matrix);
	cout<<"\nQ:\n";
	print_array(q, stp_size);

	//generate Q array by matrixB
	int *q_by_matrixB = mdr_generate_q_by_matrixB(stp_gp, p, k, encoding_matrix_B);
	cout<<"\nQ by matrixB:\n";
	print_array(q_by_matrixB, stp_size);

	//show repair_stripeIndex
	cout<<"\nshow repair stripeIndex"<<endl;
	for(int i = 1; i <= k+1; i++){
		int *R = mdr_repair_stripeIndex_by_matrixB(k, i);
		print_R(R, k);
		if(R != NULL)
			delete []R;
	}

	//repair_stripeIndex
	int *R = mdr_repair_stripeIndex_by_matrixB(k, fail_disk_id);
	cout<<"\nrepair stripeIndex of failID: "<<fail_disk_id<<"\n";
	print_R(R, k);

	//fail one disk	
	clear_row(stp_gp, p, k, fail_disk_id);
	cout<<"\nafter fail one disk:\n";
	print_2d_matrix(stp_gp, k, stp_size);
	print_array(p, stp_size);
	//print_array(q, stp_size);
	print_array(q_by_matrixB, stp_size);


	//repair one disk
	mdr_repair_one_dataDisk(stp_gp, p, q, fail_disk_id, R, k, encoding_matrix_B);



	if(R != NULL) 
		delete []R;
	mdr_delete_3d_matrix(encoding_matrix, k);
	mdr_delete_3d_matrix_B(encoding_matrix_B, k);

	for(int i = 0; i < k; i++)
		delete []stp_gp[i];
	delete []stp_gp;

	delete []p;
	delete []q;

	return 0;
}

