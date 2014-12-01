#ifndef _ALLOC_H__
#define _ALLOC_H__

#include <cstdlib>

namespace miniSTL{
	/*
	**�ռ������������ֽ�Ϊ��λ�����ڴ�
	**�������ṩ���ڲ�ʹ�ã����ṩ���ⲿʹ��
	*/

	class alloc{
	private :
		enum EAlign{ALIGN = 8}; //С�������ϵ��߽�
		enum EMaxBytes{MAXBYTES = 128}; //С����������ޣ���������malloc��������
		enum ENFreeLists{ NFREELISTS = (EMaxBytes::MAXBYTES / EAlign::ALIGN) };
		enum ENObjs {NOBJS = 20};

	private:
		//free-lists�Ľڵ㹹��
		union obj{
			union obj *next;
			char client[1];
		};

		static obj *free_list[ENFreeLists::NFREELISTS];

	private:
		static char *start_free;	//�ڴ�ؿ�ʼ��λ��
		static char *end_free;		//�ڴ�ؽ�����λ��
		static size_t heap_size;

	private:
		static size_t ROUND_UP(size_t bytes){				//���ֽ����ϵ���8�ı���
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


	//alloc���ʵ�ֲ���
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
		if (list){							//���list�л��пռ�
			free_list[index] = list->next;
			return list;
		}
		else{								//���û�пռ�
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

		char *chunk = chunk_alloc(bytes, nobjs);					//���ڴ���з���nobjs���ڴ棬ÿ���ڴ��СΪbytes
		obj **my_free_list = 0;
		obj *result = 0;
		obj *current_obj = 0, *next_obj = 0;

		if (nobjs == 1){
			return chunk;
		}
		else{
			my_free_list = free_list + FREELIST_INDEX(bytes);		//���ݿ�Ĵ�С�õ����������λ��
			result = (obj *)(chunk);								//���صĿ�ĵ�ַ
			*my_free_list = next_obj = (obj*)(chunk + bytes);		

			for (int i = 1;; ++i){									//��ʣ�µĿ���뵽������
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
	�������з���nobjnobjs����ռ䣬ÿ��Ĵ�СΪbytes�����������������һ������Ҫ������価���ܶ�Ŀ飬���޸�nobjs��ֵ��
	��������еĿ���������㣬����ڴ��з���2������ռ��С�Ŀռ䣬����ʣ��Ŀռ���������С�
	*/
	char *alloc::chunk_alloc(size_t bytes, size_t &nobjs){
		char *result = 0;
		size_t total_bytes = bytes * nobjs;						//�ܹ���Ҫ���ֽ���
		size_t bytes_left = end_free - start_free;				//�ڴ����ʣ����ڴ�

		if (bytes_left >= total_bytes){							//����ڴ����ʣ����ڴ����������Ҫ���ڴ�
			result = start_free;
			start_free = start_free + total_bytes;
			
			return result;
		}
		else if(bytes_left >= bytes){							//����ڴ���е��ڴ治��nobjs���飬���ǹ�����һ����
			nobjs = bytes_left / bytes;							//�����ʣ����ڴ��������Ŀ���
			total_bytes = nobjs * bytes;
			result = start_free;
			start_free = start_free + total_bytes;

			return result;
		}
		else{													//ʣ��Ŀռ䲻��һ���죬��Ҫ��������ռ�
			size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);	//��Ҫ������ֽ���
			if (bytes_left > 0){								//������Լ�ʣ�࣬��ô��ʣ����Լ����뵽�����У���ֹ�˷ѡ�
				obj **my_free_list = free_list + FREELIST_INDEX(bytes_left);
				((obj*)start_free)->next = *my_free_list;
				*my_free_list = (obj*)start_free;
			}
			start_free = (char *)malloc(bytes_to_get);			//���ڴ�����ռ䡣
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