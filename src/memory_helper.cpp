#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <string>

using namespace std;

#ifdef USE_MEMORY_HELPER

struct record {
	void* addr;
	int size;
	string func;
	string file;
	int line;
};

static unordered_map<void*, record> records_;
static mutex mtx_;
static size_t usage_;

//分配内存并添加记录
void* memory_record_malloc(size_t sz, const char* func, const char* file, int line) {
	void* data = malloc(sz);
	if (data) {		
		record rec;
		rec.addr = data;
		rec.file = file;
		rec.func = func;
		rec.line = line;
		rec.size = (int)sz;
		lock_guard<mutex> lock(mtx_);
		records_[data] = rec;
		usage_ += sz;
	}
	return data;
}

//释放内存并释放分配记录
void memory_record_free(void* ptr) {
	if (ptr) {		
		free(ptr);
		lock_guard<mutex> lock(mtx_);
		auto it = records_.find(ptr);
		if (it != records_.end()) {
			size_t sz = it->second.size;
			usage_ -= sz;
			records_.erase(it);
		}
	}
}

//清除所有分配的记录
void memory_record_clear() {
	lock_guard<mutex> lock(mtx_);
	records_.clear();	
}

//打印未释放的分配记录
void memory_record_print() {
	lock_guard<mutex> lock(mtx_);
	if (records_.size() == 0) {
		printf("no memory record\n");
	}
	else {
		for (auto it : records_) {
			record& rec = it.second;
			printf("memory (%d bytes) at %s %s:%d\n", rec.size, rec.func.c_str(), rec.file.c_str(), rec.line );
		}
	}
}

size_t memory_usage() {
	return usage_;
}

#endif
