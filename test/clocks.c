/* Copyright (C) 2016-2017 churuxu 
 * https://github.com/churuxu/xfoundation
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "clocks.h"
#include <time.h>

#if defined(__APPLE__)  //ios & mac init clock_serv_t once
#include <mach/clock.h>
#include <mach/mach_host.h>
#elif defined(_WIN32)  //win32
#include <windows.h>
#endif



uint64_t clock_get_tick() {

#if defined(__APPLE__)  //ios & mac
	static clock_serv_t apple_clock_service_;
	mach_timespec_t ts;
	if (!apple_clock_service_) {
		host_get_clock_service(mach_host_self(), 0, &apple_clock_service_);
	}	
	if (0 == clock_get_time(apple_clock_service_, &ts)) {
		return ((uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL);
	}
	return 0;

#elif defined(__linux__)  //linux & android etc.
	struct timespec ts;
	if (0 == clock_gettime(CLOCK_MONOTONIC, &ts)) {
		return ((uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL);
	}
	return 0;

#elif defined(_WIN32)  //win32
	static LARGE_INTEGER nTickPerSecond_;
	LARGE_INTEGER nTick;
	if (!nTickPerSecond_.QuadPart) {
		QueryPerformanceFrequency(&nTickPerSecond_);
	}
	if (nTickPerSecond_.QuadPart && QueryPerformanceCounter(&nTick)) {
		uint64_t nSecond = nTick.QuadPart / nTickPerSecond_.QuadPart;
		uint64_t nMod = nTick.QuadPart % nTickPerSecond_.QuadPart;
		uint64_t nMillSecond = nMod * 1000ULL / nTickPerSecond_.QuadPart;
		return nSecond * 1000LL + nMillSecond;
	}
	return 0;

#else  
	//WARNING: clock() in 32bit application may overflow 
	return (uint64_t)(clock() / (CLOCKS_PER_SEC / 1000));
#endif

}



uint64_t clock_get_timestamp() {

#if defined(__APPLE__)  //ios & mac
	static clock_serv_t apple_clock_service_;
	mach_timespec_t ts;
	if (!apple_clock_service_) {
		host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &apple_clock_service_);
	}
	if (0 == clock_get_time(apple_clock_service_, &ts)) {
		return ((uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL);
	}
	return 0;

#elif defined(__linux__)  //linux & android etc.
	struct timespec ts;
	if (0 == clock_gettime(CLOCK_REALTIME, &ts)) {
		return ((uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL);
	}
	return 0;

#elif defined(_WIN32)  //win32
	FILETIME ft;
	uint64_t t;
	GetSystemTimeAsFileTime(&ft);
	t = (uint64_t)ft.dwHighDateTime << 32 | ft.dwLowDateTime;	
	return (t / 10000 - 11644473600000ULL);
#else  	
	return (uint64_t)(time(NULL) * 1000);
#endif

}


