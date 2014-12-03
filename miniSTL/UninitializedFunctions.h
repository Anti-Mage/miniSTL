#ifndef __UNINITIALIZEDFUNCTIONS_H__
#define __UNINITIALIZEDFUNCTIONS_H__
#define _SCL_SECURE_NO_WARNINGS

#pragma warning(disable:4996)

#include "Construct.h"
#include "Iterator.h"
#include "TypeTraits.h"


namespace miniSTL{

	/********************************************************copy***************************************************************/
	template<class InputIterator, class ForwardIterator>
	ForwardIterator _uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, _true_type);

	template<class InputIterator, class ForwardIterator>
	ForwardIterator _uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, _false_type);

	template<class InputIterator, class ForwardIterator>
	ForwardIterator uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result){
		typedef typename _type_traits<iterator_traits<InputIterator>::value_type>::is_POD_type is_POD_type;

		return _uninitialized_copy_aux(first, last, result, is_POD_type());
	}

	template<class InputIterator, class ForwardIterator>
	ForwardIterator _uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, _true_type){
		memcpy(result, first, (last - first) * sizeof(*first));			//如果只POD数据，则直接进行内存拷贝

		return result + (last - first);									//返回表示拷贝的数据最后一个数据后面一位的迭代器
	}

	template<class InputIterator, class ForwardIterator>
	ForwardIterator _uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, _false_type){
		int i = 0;
		for (; first != last; ++first, ++i){
			construct(result + i, *first);
		}

		return (result + i);
	}

	/*******************************************fill***************************************************************/

	template<class ForwardIterator, class T>
	void _uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& value, _true_type){
		std::fill(first, last, value);
	}

	template<class ForwardIterator, class T>
	void _uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& value, _false_type){

		for (; first != last; ++first, ++i){
			construct(first, value);					//逐个构造
		}
	}

	template<class ForwardIterator, class T>
	void uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& value){
		typedef typename _type_traits<T>::is_POD_type is_POD_type;
		_uninitialized_fill_aux(first, last, value, is_POD_type());
	}


	/******************************************n_fill*******************************************************************/
	template<class ForwardIterator, class Size, class T>
	ForwardIterator _uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& x, _true_type){
		ForwardIterator last = first + n;
		uninitialized_fill(first, last, x);
		return last;
	}

	template<class ForwardIterator, class Size, class T>
	ForwardIterator _uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& x, _false_type){
		int i = 0;
		for (; i != n; ++i){
			construct(first + i, x);
		}

		return (first + i);
	}

	template<class ForwardIterator, class Size, class T>
	ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n, const T& x){
		typedef typename _type_traits<T>::is_POD_type is_POD_type;

		return _uninitialized_fill_n_aux(first, n, x, is_POD_type());
	}

}


#endif