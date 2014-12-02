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
			T* getPtr(){ return ptr_; }
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
			viter operator-(const difference_type i){ return viter(ptr_ - i)_; }
			viter operator+(const difference_type i){ return viter(ptr_ + i); }
			difference_type operator-(const viter& v){ return (ptr_ - v.ptr_); }

			friend viter operator+(const viter& vit, const difference_type i);
			friend viter operator+(const difference_type i, const viter& vit);
			friend viter operator-(const viter& vit, const difference_type i);
			friend viter operator-(const difference_type i, const viter& vit);
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
		viter<T> operator+(const viter<T>& vit, const typename viter<T>::difference_type i){
			return vit + i;
		}

		template<class T>
		viter<T> operator+(const typename viter<T>::difference_type i, const viter<T>& vit){
			return vit + i;
		}

		template<class T>
		viter<T> operator-(const viter<T>& vit, const typename viter<T>::difference_type i){
			return vit - i;
		}

		template<class T>
		viter<T> operator-(const typename viter<T>::difference_type i, const viter<T>& vit){
			return vit - i;
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
		vector() :strat_(0), finish_h(0), endofStorage_(0){}
		explicit vector(const size_type n);
		vector(const size_type n,const value_type &value);
		template<class InputIterator>
		vector(InputIterator first, InputIterator last);
		vector(const vector &v);
		vector(vector &&v);
		vector& operator=(const vector& v);
		~vector(){
			dataAllocator::destroy(start_, finish_);
			dataAllocator::deallocate(start_, endofStorage_ - start_);
		}

		iterator begin(){ return iterator(start_); }
		iterator end(){ return iterator(finish_); }
		difference_type size() const { return finish_ - start_; }
		difference_type capacity() const{ return endofStorage_ - start_; }
		bool empty() { return start_ == finish_; }

		value_type& operator[](const difference_type i){ return *(begin() + i); }
		value_type& front(){ return *(begin()); }
		value_type& back(){ return *(--end()); }
		pointer data(){ return start_; }

		void clear(){
			dataAllocator::destroy(start_, finish_);
			finish_ = start_;
		}

		void swap(vector& v){
			if (this != &v){
				std::swap(start_, v.start_);
				std::swap(finish_, v.finish_);
				std::swap(endofStorage_, v.endofStorage_);
			}
		}

		void push_back(const value_type& value);
		void pop_back(){
			--finish_;
			dataAllocator::deallocate(finish_);
		}

		iterator insert(iterator position, const value_type& value);
		void insert(iterator position, size_type n, const value_type& value);
		template<class InputIterator>
		void insert(iterator position, InputIterator first, InputIterator last);

		iterator erase(iterator position);
		iterator erase(iterator first, iterator last);

	private:
		void allocateAndFillN(const size_type n, const value_type& value){
			start_ = dataAllocator::allocate(n);
			miniSTL::uninitialized_fill_n(start_, n, value);
			finish_ = start_ + n;
			endofStorage_ = finish_;
		}

		template<class InputIterator>
		void allocateAndCopy(InputIterator first, InputIterator last){
			start_ = dataAllocator::allocate(last - first);
			finish_ = uninitialized_copy(first, last, start_);
			endofStorage_ = finish_;
		}

		template<class InputIterator>
		void vector_aux(InputIterator first, InputIterator last, _false_type){
			allocateAndCopy(first, last);
		}

		template<class InputIterator>
		void vector_aux(InputIterator first, InputIterator last, _true_type){
			allocateAndCopy(first, last);
		}


	};

	template<class T, class Alloc>
	vector<T, Alloc>::vector(size_type n){
		allocateAndFillN(n, value_type());
	}

	template<class T, class Alloc>
	vector<T, Alloc>::vector(const size_type n,const value_type &value){
		allocateAndFillN(n, value);
	}

	template<class T, class Alloc>
	template<class InputIterator>
	vector<T, Alloc>::vector(InputIterator first, InputIterator last){
		typename _type_traits< iterator_traits<InputIterator>::value_type>::is_POD_type is_POD_type;
		vector_aux(first, last, is_POD_type);
	}

	template<class T, class Alloc>
	vector<T, Alloc>::vector(const vector &v){
		allocateAndCopy(v.begin(), v.end());
	}

	template<class T, class Alloc>
	vector<T, Alloc>::vector(vector&& v){
		start_ = v.start_;
		finish_ = v.finish_;
		endofStorage_ = v.endofStorage_;
		v.clear();
	}

	template<class T, class Alloc>
	vector<T, Alloc>& vector<T, Alloc>::operator=(const vector& v){
		if (this != &v){
			allocateAndCopy(v.start_, v.finish_);

		}

		return *this;
	}

	template<class T, class Alloc>
	typename vector<T, Alloc>::iterator vector<T, Alloc>::erase(iterator position){
		return erase(position, position + 1);
	}

	template<class T, class Alloc>
	typename vector<T, Alloc>::iterator vector<T, Alloc>::erase(iterator first, iterator last){
		difference_type lenOfTail = end() - last;
		difference_type lenOfRemoved = last - first;
		finish_ = finish_ - lenOfRemoved;
		for (; lenOfTail != 0; --lenOfTail){
			auto tmp = (last - lenOfTail);
			*tmp = *(last++ )
		}

		return viter<T>(first);
	}
}

#endif