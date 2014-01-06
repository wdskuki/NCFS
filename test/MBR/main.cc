/*
 * main.cc
 * - testing program
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <time.h>
//#include "netcodec/netcodec.hh"
#include "netcodec.hh"

int main() {  
	NetCodec* nc;  
	CodeBlock* cb;
	uint8_t data[4];       //the data in each block is size of 4 bytes.
	printf("\n");printf("The size of each block is defined as 4 bytes, represented as 4 integer-symbols (less than 128) here. ");printf("\n"); 
	uint8_t ret_data[1500];
	int i, j;

  //0.MBR factors: MBR_N , MBR_K, MBR_original_block_num, MBR_encoded_block_num
  int MBR_N=10;
	int MBR_K=5;
	int MBR_original_block_num = MBR_K*(2*MBR_N-MBR_K-1)/2 ;
  int MBR_encoded_block_num = MBR_N*(MBR_N-1)/2 - MBR_K*(2*MBR_N-MBR_K-1)/2 ;
  int MBR_all_block_num = MBR_N*(MBR_N-1)/2 ;
  printf("\nMDS(%d,%d): There are %d nodes to store a file.",MBR_N,MBR_K,MBR_N);
  printf(" Any %d of %d nodes can rebuild the file.\n", MBR_K,MBR_N);
	printf("\nThe file is divided as %d blocks based on MBR.",MBR_original_block_num);
  printf("\nThe original blocks generate %d encoded block with random linearly coding.\n ",MBR_encoded_block_num );
  printf("\nEach node needs to store %d blocks based on MBR.",MBR_N-1);
  
	//1.File Division: The file is divided into MBR_original_block_num blocks.
	printf("\n\n*************************  Intially  *************************\n");
	nc = new NetCodec(MBR_original_block_num,MBR_encoded_block_num); 
	for (i=0; i<MBR_original_block_num; ++i){	
		printf("The original block %d: [ ",i+1);
		for (j=0; j<4; ++j){			
			data[j] = i % 100+1;
			printf("%u ", data[j]);		
		}
		printf("]\n");
		nc->addNativePacket(data, 4);	
	}

	//2. Block Encoding: The file is encoded into MBR_encoded_block_num blocks.
	srand((unsigned)time(0));
	printf("\n\n*************************  Blocks Encoding  ************************* ");
	for (i=0; i<MBR_encoded_block_num; ++i) {
		cb = nc->encode();	
		printf("\nThe encoded block %d :\n",i+1); 
		cb->print(); 
		nc->store_encode(cb);		 //store the encoded blocks.
	}
	

	//3.Blocks Placement: place all the blocks (original and encoded ones )into MBR_N storage nodes
  //3.1 Define an all_block_list to store all the blocks. 
  //Put all the original blocks and encoded blocks into the all_block_list.
  vector <CodeBlock*> all_block_list; 
 	for (i=0;i<MBR_all_block_num;i++){
		if (i<MBR_original_block_num ){
			all_block_list.push_back(nc->orig_matrix_block(i)); 
		}
		else{
			all_block_list.push_back(nc->enc_matrix_block(i-MBR_original_block_num)); 
		}
	}
	//3.2 Define all the data in MBR_N storage nodes as node_data,
  //where each node contains some blocks which are represented as a vector.
	vector< vector<CodeBlock*> > node_data( MBR_N, vector< CodeBlock* >(NULL) );
	//3.3 Distribute all the blocks into n storage nodes
	int index = 0;
  for (i=0;i<MBR_N;i++){
		for (j=0;j<MBR_N-1-i;j++){
			node_data[i].push_back (all_block_list[index++]); 
		}
	}
	index = 0;
  for (j=1;j<MBR_N;j++){					
		for (i=j;i<MBR_N;i++){						
			node_data[i].push_back (all_block_list[index++]);					
			}
	}
	printf("\n");printf("*************************  Blocks Placement  ************************* ");
	for (i=0;i<MBR_N;i++){
		printf("\nnode %d stores %d blocks below: \n", i+1,MBR_N-1);
		for (j=0;j<MBR_N-1;j++){			
			node_data[i][j]->print();
		}
	}

  //4 File Reconstruction
  //4.1.Define avail_node_index_set = {0,1,...,MBR_N-1} as the index set of 
  //MBR_N available nodes {node 0, node 1, ..., node MBR_N-1}.
  vector <int>avail_node_index_set (MBR_N); 
  for (i=0;i<MBR_N;i++){
  	avail_node_index_set.push_back(i);
	}
	//4.2.Define select_node_index_set as the index set of MBR_K selected nodes 
	//for file reconstruction. 
	vector <int>select_node_index_set (MBR_K);
	for (i=0;i<MBR_K;i++){
  	select_node_index_set.push_back(-1);
	}
	//4.3. Construct the select_node_index_set from avail_node_index_set,
	//such as, select_node_index_set:{0,1,4} from avail_node_index_set:{0,1,2,3,4} based on MDS-(5,3).
  int select_node_index = -1;
  int select_node_num = 0;
  while (select_node_num<MBR_K){
		select_node_index = rand()%(MBR_N);
		if(avail_node_index_set[select_node_index]!=-1){
			select_node_index_set[select_node_num]=select_node_index;			
			avail_node_index_set[select_node_index]=-1;
			select_node_num++;
		}
	}

	//4.4.Use the content of the selected nodes to construct the decoding matrix.
  for (i=0;i<MBR_K;i++){ 
  	for (j=0;j<MBR_N-1;j++){
  		nc->decode(node_data[select_node_index_set[i]][j]);
  	}
  }  
  //4.5.Use the decoding matrix to rebuild the original file.

  printf("\n\n");printf("*************************  File Rebuilding  ************************* ");
  printf("\nWe select ");
	for (i=0;i<MBR_K;i++){
		printf(" (node %d) ",select_node_index_set[i]+1);
	}
	printf("to rebulid the file.\n");
  j=0;
	while (nc->isDecodable()) {
		nc->getNativePacket(ret_data);
		printf("\nThe rebuilt block %d : [  ",++j);
		for (i=0; i<4; ++i) {
			printf("%d ", ret_data[i]);
		}
		printf(" ]\n");
	}		
	return 0;
}
