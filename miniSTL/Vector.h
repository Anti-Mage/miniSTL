#ifndef _VECTOR_H__ 
#define _VECTOR_H__

#include "Allocator.h"
#include "Iterator.h"
#include "UninitializedFunctions.h"

namespace miniSTL{
	
	namespace {
		template<class T>
		class viter : public iterator < random_access_iterator<T, ptrdiff_t>, T > {
		private :
			T *ptr_;
		public:
			viter() :ptr_(0){}
			explicit viter(T *ptr) : ptr_(ptr){}
			viter(const viter& vit);
			viter& operator=(const viter &vit);

			T& operator*(){ return *ptr_; }
			T* operator->(){ return ptr_; }

			viter& operator++(){
				++ptr_; 
				return *this;
			}
			viter& operator++(int){
				viter tmp = *this;
				++(*this);
				return tmp;
			}

			viter& operator--(){
				--ptr_;
				return *this;
			}
			viter& operator--(int){
				viter tmp = *this;
				--(*this);
				return tmp;
			}

			bool operator==(const viter& vit){return ptr_ == vit.ptr_}
			bool operator!=(const viter& vit){ return !(*this == vit) }
			typename viter::difference_type operator-(const viter& vit){ return ptr_ - vit.ptr_; }
			friend typename viter::difference_type operator-(const viter& lhs, const viter& rhs);
		};

		template<class T>
		viter<T>::viter(const viter& vit){
			ptr_ = vit.ptr_;
		}

		template<class T>
		viter<T>& viter<T>::operator=(const viter& rhs){
			if (this != &rhs){			//只用在两个变量不同的情况下才赋值
				ptr_ = rhs.ptr_;
			}
		}

		template<class T>
		typename viter::difference_type operator-(const viter& lhs, const viter& rhs){
			return (lhs - rhs);
		}
	}		//end of anonymous namespace

	template<class T, class Alloc = allocator<T>>
	class vector{
	private:
		T *start_;
		T *finish_;
		T *endofStorage_;

		typedef Alloc dataAllocator;

	public:
		typedef T										value_type;
		typedef viter<T>								iterator;
		typedef iterator								pointer;
		typedef T&										reference;
		typedef size_t									size_type;
		typedef typename iterator::difference_type		difference_type;

	public:
	};
}

#endif