#include <iostream>

#include "Construct.h"
#include "Alloc.h"
#include "Allocator.h"
#include "Iterator.h"


using namespace std;

int main(){
	cout << "hello world!" << endl;

	for (int i = 1; i < 50; ++i){
		mySTL::alloc::allocate(i % 128 * sizeof(int));
		auto p = mySTL::allocator<int>::allocate();
		mySTL::allocator<int>::construct(p, i);
		std::cout << *p << endl;
		mySTL::allocator<int>::construct(p, i * i);
		//std::cout << *p << endl;
		mySTL::allocator<int>::destroy(p);			//得到的值之前一样
		std::cout << *p << endl;
		mySTL::allocator<int>::deallocate(p);
		//std::cout << *p << endl;					//释放内存之后得到的是随机值
	}

	return 0;
}