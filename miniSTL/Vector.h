#ifndef _VECTOR_H__ 
#define _VECTOR_H__

#include "Allocator.h"
#include "Iterator.h"
#include "UninitializedFunctions.h"

namespace miniSTL{
	
	namespace {
		template<class T>
		class viter : public miniSTL::iterator <miniSTL::random_access_iterator_tag, T > {
		private :
			T *ptr_;
			T* getPtr(){ return ptr_; }
		public:
			viter() :ptr_(0){}
			explicit viter(T *ptr) : ptr_(ptr){}
			viter(const viter& vit);
			viter& operator=(const viter &vit);

			operator void*(){ return ptr_; }

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
			destroyAndDeallocateAll();
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
		void insert(iterator position,const size_type& n, const value_type& value);
		template<class InputIterator>
		void insert(iterator position, InputIterator first, InputIterator last);

		iterator erase(iterator position);
		iterator erase(iterator first, iterator last);

	private:
		void destroyAndDeallocateAll(){
			if (capacity() != 0){
				dataAllocator::destroy(start_, finish_);
				dataAllocator::deallocate(start_, endofStorage_ - start_);
			}
		}

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
		void vector_aux(InputIterator first, InputIterator last, std::false_type){
			allocateAndCopy(first, last);
		}

		template<class Integer>
		void vector_aux(Integer n, Integer value, std::true_type){
			allocateAndFillN(n, value);
		}

		template<class InputIterator>
		void insert_aux(iterator position, InputIterator first, InputIterator last, std::false_type);
		template<class Integer>
		void insert_aux(iterator position, Integer n, Integer value, std::true_type);

		template<class InputIterator>
		void reallocateAndCopy(iterator position, InputIterator first, InputIterator last);
		void reallocateAndFillN(iterator position, const size_type& n, const value_type& val);

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
		/*typename _type_traits< iterator_traits<InputIterator>::value_type>::is_POD_type is_POD_type;*/
		vector_aux(first, last, typename std::is_integral<InputIterator>::type());
		//allocateAndCopy(first, last);
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


	template<class T, class Alloc>
	template<class InputIterator>
	void vector<T, Alloc>::reallocateAndCopy(iterator position, InputIterator first, InputIterator last){
		difference_type oldCapacity = endOfStorage_ - start_;
		oldCapacity = oldCapacity ? oldCapacity : 1;
		difference_type newCapacity = oldCapacity + std::max(oldCapacity, last - first);

		T *newStart = dataAllocator::allocate(newCapacity);
		T *newEndOfStorage = newStart + newCapacity;
		T *newFinish = uninitialized_copy(begin(), position, newStart);
		newFinish = uninitialized_copy(first, last, newFinish);
		newFinish = uninitialized_copy(position, end(), newFinish);

		destroyAndDeallocateAll();
		start_ = newStart;
		finish_ = newFinish;
		endOfStorage_ = newEndOfStorage;
	}
	template<class T, class Alloc>
	template<class InputIterator>
	void vector<T, Alloc>::insert_aux(iterator position,
		InputIterator first, InputIterator last,
		std::false_type){
		difference_type locationLeft = endOfStorage_ - finish_; // the size of left storage
		difference_type locationNeed = last - first;

		if (locationLeft >= locationNeed){
			auto tempPtr = end() - 1;
			for (; tempPtr - position >= 0; --tempPtr){//move the [position, finish_) back
				*(tempPtr + locationNeed) = *tempPtr;
			}
			uninitialized_copy(first, last, position);
			finish_ += locationNeed;
		}
		else{
			reallocateAndCopy(position, first, last);
		}
	}
	template<class T, class Alloc>
	template<class Integer>
	void vector<T, Alloc>::insert_aux(iterator position, Integer n, Integer value, std::true_type){
		vector<value_type> v(n, value);
		insert(position, v.begin(), v.end());
	}
	template<class T, class Alloc>
	template<class InputIterator>
	void vector<T, Alloc>::insert(iterator position, InputIterator first, InputIterator last){
		insert_aux(position, first, last, typename std::is_integral<InputIterator>::type());
	}
	template<class T, class Alloc>
	void vector<T, Alloc>::insert(iterator position, const size_type& n, const value_type& val){
		insert_aux(position, n, val, typename std::is_integral<value_type>::type());
	}
	template<class T, class Alloc>
	typename vector<T, Alloc>::iterator vector<T, Alloc>::insert(iterator position, const value_type& val){
		insert(position, 1, val);
		return position;
	}
	template<class T, class Alloc>
	void vector<T, Alloc>::push_back(const value_type& value){
		insert(end(), value);
	}
}

#endif