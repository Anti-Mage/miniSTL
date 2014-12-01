#ifndef _ALLOC_H__
#define _ALLOC_H__

#include <cstdlib>

namespace miniSTL{
	/*
	**空间配置器，以字节为单位分配内存
	**函数仅提供给内部使用，不提供给外部使用
	*/

	class alloc{
	private :
		enum EAlign{ALIGN = 8}; //小型区块上调边界
		enum EMaxBytes{MAXBYTES = 128}; //小型区块的上限，超过的由malloc函数分配
		enum ENFreeLists{ NFREELISTS = (EMaxBytes::MAXBYTES / EAlign::ALIGN) };
		enum ENObjs {NOBJS = 20};

	private:
		//free-lists的节点构造
		union obj{
			union obj *next;
			char client[1];
		};

		static obj *free_list[ENFreeLists::NFREELISTS];

	private:
		static char *start_free;	//内存池开始的位置
		static char *end_free;		//内存池结束的位置
		static size_t heap_size;

	private:
		static size_t ROUND_UP(size_t bytes){				//将字节数上调至8的倍数
			return ((bytes + EAlign::ALIGN - 1) & ~(EAlign::ALIGN - 1));
		}

		static size_t FREELIST_INDEX(size_t bytes){
			return (((bytes)+EAlign::ALIGN - 1) / EAlign::ALIGN - 1);
		}

		static void *refill(size_t n);

		static char *chunk_alloc(size_t size, size_t &nobjs);

	public:
		static void *allocate(size_t bytes);
		static void deallocate(void *ptr, size_t bytes);
		static void *reallocate(void *ptr, size_t old_sz, size_t new_sz);
	};


	//alloc类的实现部分
	char *alloc::start_free = 0;
	char *alloc::end_free = 0;
	size_t alloc::heap_size = 0;
	alloc::obj *alloc::free_list[alloc::ENFreeLists::NFREELISTS] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	void *alloc::allocate(size_t bytes){
		if (bytes > EMaxBytes::MAXBYTES){
			return malloc(bytes);
		}
		size_t index = FREELIST_INDEX(bytes);
		obj *list = free_list[index];
		if (list){							//如果list中还有空间
			free_list[index] = list->next;
			return list;
		}
		else{								//如果没有空间
			return refill(ROUND_UP(bytes));
		}
	}

	void alloc::deallocate(void *ptr, size_t bytes){
		if (bytes > EMaxBytes::MAXBYTES)
			free(ptr);
		else{
			size_t index = FREELIST_INDEX(bytes);
			obj *node = static_cast<obj *>(ptr);
			node->next = free_list[index];
			free_list[index] = node;
		}
	}

	void *alloc::reallocate(void *ptr, size_t old_sz, size_t new_sz){
		deallocate(ptr, old_sz);
		ptr = allocate(new_sz);

		return ptr;
	}

	//
	void *alloc::refill(size_t bytes){
		size_t nobjs = ENObjs::NOBJS;

		char *chunk = chunk_alloc(bytes, nobjs);					//从内存池中分配nobjs块内存，每块内存大小为bytes
		obj **my_free_list = 0;
		obj *result = 0;
		obj *current_obj = 0, *next_obj = 0;

		if (nobjs == 1){
			return chunk;
		}
		else{
			my_free_list = free_list + FREELIST_INDEX(bytes);		//根据块的大小得到块在链表的位置
			result = (obj *)(chunk);								//返回的块的地址
			*my_free_list = next_obj = (obj*)(chunk + bytes);		

			for (int i = 1;; ++i){									//将剩下的块加入到链表中
				current_obj = next_obj;
				next_obj = (obj *)((char *)next_obj + bytes);

				if (nobjs - 1 == i){
					current_obj->next = 0;
					break;
				}
				else{
					current_obj->next = next_obj;
				}
			}
		}
		return result;
	}

	/*
	从链表中分配nobjnobjs块个空间，每块的大小为bytes。如果链表中至少有一个满足要求，则分配尽可能多的块，并修改nobjs的值。
	如果链表中的块的数量不足，则从内存中分配2倍所需空间大小的空间，并将剩余的空间加入链表中。
	*/
	char *alloc::chunk_alloc(size_t bytes, size_t &nobjs){
		char *result = 0;
		size_t total_bytes = bytes * nobjs;						//总共需要的字节数
		size_t bytes_left = end_free - start_free;				//内存池中剩余的内存

		if (bytes_left >= total_bytes){							//如果内存池中剩余的内存可以满足需要的内存
			result = start_free;
			start_free = start_free + total_bytes;
			
			return result;
		}
		else if(bytes_left >= bytes){							//如果内存池中的内存不够nobjs个块，但是够至少一个块
			nobjs = bytes_left / bytes;							//计算出剩余的内存可以满足的块数
			total_bytes = nobjs * bytes;
			result = start_free;
			start_free = start_free + total_bytes;

			return result;
		}
		else{													//剩余的空间不够一个快，需要重新申请空间
			size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);	//需要申请的字节数
			if (bytes_left > 0){								//如果有自己剩余，那么将剩余的自己加入到链表中，防止浪费。
				obj **my_free_list = free_list + FREELIST_INDEX(bytes_left);
				((obj*)start_free)->next = *my_free_list;
				*my_free_list = (obj*)start_free;
			}
			start_free = (char *)malloc(bytes_to_get);			//从内存申请空间。
			if (!start_free){
				obj **my_free_list = 0, *p = 0;
				for (int i = 0; i <= EMaxBytes::MAXBYTES; i += EAlign::ALIGN){
					my_free_list = free_list + FREELIST_INDEX(i);
					p = *my_free_list;
					if (!p){
						*my_free_list = p->next;
						start_free = (char*)p;
						end_free = start_free + i;
						return chunk_alloc(bytes, nobjs);
					}
				}
				end_free = 0;
			}
			heap_size += bytes_to_get;
			end_free = start_free + bytes_to_get;

			return chunk_alloc(bytes, nobjs);
		}
	}
}

#endif