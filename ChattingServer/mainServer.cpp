#include "pch.h"
#include "TlsProfiler.h"
#include "MemoryPool.h"

#include <vector>

using namespace std;
int main()
{
	vector<int> v{ 1,2 };

	vector<int>v2;
	v2.push_back(std::move(v[1]));
	int a = 3;
}