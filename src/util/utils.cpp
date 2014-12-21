/*
 * utils.cpp
 *
 *  Created on: 1 oct. 2014
 *      Author: jfellus
 */

#include "utils.h"
#include <glib-2.0/glib-object.h>
#include <execinfo.h>
#include <sys/time.h>

#pragma GCC diagnostic ignored "-Wdeprecated-declarations"


DebugStream* DEBUG_STREAM = new DebugStreamStd();
DebugStream* ERROR_STREAM = new DebugStreamStd();


static bool utils_init();
static bool inited = utils_init();
bool utils_init() {
	(void) inited;
	g_type_init();
	return true;
}


void PRINT_STACK_TRACE() {
	void *array[10];
	size_t size = backtrace(array, 10);
	backtrace_symbols_fd(array, size, STDERR_FILENO);
}

std::string GET_STACK_TRACE() {
	void *array[10];
	size_t size = backtrace(array, 10);
	char** s = backtrace_symbols(array, size);
	std::string str = "";
	for(uint i=0; i<size; i++) {
		if(s[i]) str += s[i]; str += "\n";
	}
	return str;
}

std::string get_cur_thread_name() {
	char name[1000]; pthread_getname_np(pthread_self(), name, 1000);
	return name;
}


static struct timeval tstart, tend;
void TIC() {
	gettimeofday(&tstart, NULL);
}

void TAC() {
	gettimeofday(&tend, NULL);
	float ms = (tend.tv_sec - tstart.tv_sec)*1000.0 + (tend.tv_usec - tstart.tv_usec)/1000.0;
	float fps = 1000.0 / ms;
	DBG(ms << "ms " << fps << "fps");
}

long get_time_seconds() {
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec;
}


