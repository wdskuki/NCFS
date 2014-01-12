#include <math.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
using namespace std;

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

int* mdr_repair_p(int **stp_gp, int k){
	return mdr_generate_p(stp_gp, k);
}

int* mdr_repair_q(int **stp_gp, int k, int ***encoding_matrix){
	return mdr_generate_q(stp_gp, k, encoding_matrix);
}

int* mdr_repair_data(int **stp_gp, int *q,
					 int k, int fail_disk_id, 
					 int ***encoding_matrix){
	
	int sg_row = k;
	int sg_col = (int)pow(2, k);
	int m_row = sg_col;
	int m_col = m_row;

	int *new_comer = new int [sg_col];

	for(int j = 0; j < m_row; j++) new_comer[j] = 0;

	for(int j = 0; j < m_row; j++){
		for(int i = 0; i < k; i++){
			for(int t = 0; t < m_col; t++){
				//TODO:
			}
		}
	}
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

void clear_row(int **stp_gp, int k, int row_index){
	int row = k;
	int col = (int)pow(2, k);
	for(int i = 0; i < col; i++)
		stp_gp[row_index][i] = -1;
}

int main(int argc, char const *argv[])
{
	
	if(argc != 2){
		cout<<"error inputs\n";
		return -1;
	}

	int k = atoi(argv[1]);
	int stp_size = (int)pow(2,k);

	//generate encoding_matrix
	int ***encoding_matrix = mdr_encoding_matrix(k);
	cout<<"\nencoding_matrix:\n";
	mdr_print_matrix(encoding_matrix, k);

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

	//fail one disk
	int fail_disk_id = 1;
	clear_row(stp_gp, k, fail_disk_id);
	cout<<"\nafter fail one disk:\n";
	print_2d_matrix(stp_gp, k, stp_size);
	print_array(p, stp_size);
	print_array(q, stp_size);

	//repair one disk



	mdr_delete_3d_matrix(encoding_matrix, k);
	
	for(int i = 0; i < k; i++)
		delete []stp_gp[i];
	delete []stp_gp;

	delete []p;
	delete []q;

	return 0;
}

