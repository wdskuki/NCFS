#include <vector>
#include <iostream>
using namespace std;
int main(int argc, char const *argv[])
{
	vector<int> ivec;
	ivec.push_back(11);

	vector<int> ivec2 = ivec;

	for(int i = 0; i < ivec2.size(); i++){
		cout<<ivec2[i]<<endl;
	}
	return 0;
}