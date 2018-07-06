#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <string>


using namespace std;


#ifdef USE_MEMORY_TLSPOOL


class tlspool {
public:
	tlspool() {
		tlen = 0;
		rlen = 0;
		ptr = NULL;
		buffer = NULL;
		last = NULL;
	}
	~tlspool() {
		deinit();
	}

	bool valid() {
		return (buffer != NULL);
	}

	bool isowner(char* ptr) {
		if (buffer && ptr >= buffer && ptr < (buffer + tlen)) {
			return true;
		}
		return false;
	}

	void init(size_t sz) {
		if (!buffer) {
			buffer = (char*)malloc(sz);			
		}
		if (buffer) {
			tlen = sz;
			rlen = sz;
			ptr = buffer;
			last = NULL;
		}
	}

	void deinit() {
		if (buffer) {
			free(buffer);
		}
	}

	void* alloc(size_t sz) {
		void* ret = NULL;
		if (rlen >= sz) {
			rlen -= sz;
			ret = ptr;
			last = ptr;
			ptr += sz;
			memset(ret, 0, sz);
		}
		return ret;
	}

	void release(char* p) {
		if (p == last) {
			rlen -= (ptr - last);
			ptr = last;
			last = NULL;			
		}
	}

private:
	char* buffer; //缓冲区
	size_t tlen; //总长度
	size_t rlen;  //剩余长度 
	char* ptr; //下一个地址
	char* last; //上一次分配的地址
};

static thread_local tlspool pool_;
static size_t total_;

void* memory_tlspool_malloc(size_t sz) {
	if (pool_.valid()) {
		return pool_.alloc(sz);
	}
	else {
		return malloc(sz);
	}
}


void memory_tlspool_free(void* ptr) {
	if (!ptr)return;
	if (pool_.isowner((char*)ptr)) {
		pool_.release((char*)ptr);
	}
	else {
		free(ptr);
	}
}

void memory_tlspool_init(size_t total) {
	total_ = total;
}

void memory_tlspool_reset() {
	pool_.init(total_);

}

#endif


