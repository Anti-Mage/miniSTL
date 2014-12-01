#ifndef _CONSTRUCT_H__
#define _CONSTRUCT_H__

#include <new>
#include "TypeTraits.h"


namespace mySTL{

	template<class T1, class T2>
	inline void construct(T1 *ptr1, const T2& value){
		new (ptr1)T1(value);
	}

	template<class T>
	inline void destroy(T *ptr){
		ptr->~T();
	}

	template<class ForwardInterator>
	inline void _destroy(ForwardInterator first, ForwardInterator last, _true_type){

	}

	template<class ForwardInterator>
	inline void _destroy(ForwardInterator first, ForwardInterator last, _false_type){
		for (; first != last; ++first)
			destroy(&*first);
	}

	template<class ForwardInterator>
	inline void destroy(ForwardInterator first, ForwardInterator last){
		typedef typename _type_traits<T>::is_POD_type is_POD_type;
		_destroy(first, last, is_POD_type);
	}
}
#endif