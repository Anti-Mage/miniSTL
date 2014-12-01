#include <iostream>
#include <iterator>

#include "Construct.h"
#include "Alloc.h"
#include "Allocator.h"
#include "Iterator.h"
#include "UninitializedFunctions.h"


using namespace std;

int main(){
	cout << "hello world!" << endl;

	for (int i = 1; i < 50; ++i){
		//miniSTL::alloc::allocate(i % 128 * sizeof(int));
		//auto p = miniSTL::allocator<int>::allocate();
		//miniSTL::allocator<int>::construct(p, i);
		//std::cout << *p << endl;
		//miniSTL::allocator<int>::construct(p, i * i);
		//std::cout << *p << endl;
		//miniSTL::allocator<int>::destroy(p);			//得到的值之前一样
		//std::cout << *p << endl;
		//miniSTL::allocator<int>::deallocate(p);
		//std::cout << *p << endl;					//释放内存之后得到的是随机值


	}

	auto p = miniSTL::allocator<int>::allocate(100);
	miniSTL::uninitialized_fill(p, p + 90, 123);
	copy(p, p + 100, ostream_iterator<int>(cout, " ")); cout << endl;

	auto last = p + 100;
	miniSTL::uninitialized_fill_n(p, 20, 1234);
	copy(p, p + 100, ostream_iterator<int>(cout, " ")); cout << endl;

	int array[100];
	miniSTL::uninitialized_fill_n(array, 100, 100);
	copy(p, p + 100, ostream_iterator<int>(cout, " ")); cout << endl;
	for (auto n : array){
		cout << n << ' ';
	}

	return 0;
}