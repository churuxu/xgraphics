#pragma once


/*
malloc/free 内存释放检测辅助工具
*/

#ifdef USE_MEMORY_HELPER

//分配内存并添加记录
void* memory_record_malloc(size_t sz, const char* func, const char* file, int line);

//释放内存并释放分配记录
void memory_record_free(void* ptr);

//清除所有分配的记录
void memory_record_clear();

//打印未释放的分配记录
void memory_record_print();


#define malloc(sz) memory_record_malloc(sz, __FUNCTION__, __FILE__, __LINE__)
#define free memory_record_free


#endif

