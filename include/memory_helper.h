#pragma once


/*
malloc/free �ڴ��ͷż�⸨������
*/

#ifdef USE_MEMORY_HELPER

//�����ڴ沢��Ӽ�¼
void* memory_record_malloc(size_t sz, const char* func, const char* file, int line);

//�ͷ��ڴ沢�ͷŷ����¼
void memory_record_free(void* ptr);

//������з���ļ�¼
void memory_record_clear();

//��ӡδ�ͷŵķ����¼
void memory_record_print();


#define malloc(sz) memory_record_malloc(sz, __FUNCTION__, __FILE__, __LINE__)
#define free memory_record_free


#endif

