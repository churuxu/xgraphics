#pragma once


/*
malloc/free Ԥ�ȷ���
*/

#ifdef USE_MEMORY_TLSPOOL

//�����ڴ�
void* memory_tlspool_malloc(size_t sz);

//�ͷ��ڴ�
void memory_tlspool_free(void* ptr);

//����Ԥ�����ڴ��С
void memory_tlspool_init(size_t total);

//����Ԥ�����ڴ�
void memory_tlspool_reset();


#define malloc(sz) memory_tlspool_malloc(sz)
#define free memory_tlspool_free


#endif



