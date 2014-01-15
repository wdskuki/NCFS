#include <iostream>
#include <stdlib.h>
using namespace std;

int main(int argc, char const *argv[])
{
	if(argc <= 1){
		cout<<"error: xor.cc\n";
		return -1;
	}
	int res = 0;
	for(int i = 1; i <= argc-1; i++){
		int a = atoi(argv[i]);
		res = res ^ a;
	}
	cout<<res<<endl;
	return 0;
}