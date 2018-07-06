#pragma once


/*
malloc/free 预先分配
*/

#ifdef USE_MEMORY_TLSPOOL

//分配内存
void* memory_tlspool_malloc(size_t sz);

//释放内存
void memory_tlspool_free(void* ptr);

//设置预分配内存大小
void memory_tlspool_init(size_t total);

//重置预分配内存
void memory_tlspool_reset();


#define malloc(sz) memory_tlspool_malloc(sz)
#define free memory_tlspool_free


#endif



