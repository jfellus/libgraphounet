/*
 * utils.h
 *
 *  Created on: 1 oct. 2014
 *      Author: jfellus
 */

#ifndef UTILS_H_
#define UTILS_H_


#include "string.h"
#include "file.h"
#include "func_ptr.h"
#include <pthread.h>
#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <list>

class DebugStream {
public:
	std::ostream& stream;
public:
	DebugStream(std::ostream& stream) : stream(stream) {}
	virtual ~DebugStream(){}
	virtual void end() {}
};
class DebugStreamStd : public DebugStream {
public:
	DebugStreamStd() : DebugStream(std::cerr) {}
	virtual ~DebugStreamStd() {}
};
extern DebugStream* DEBUG_STREAM;
extern DebugStream* ERROR_STREAM;

#define FAIL(msg) do { fprintf (stderr, "FAIL: %s\n", msg); exit (-1); } while (0)

#define DBG(x) do {DEBUG_STREAM->stream << x << "\n"; DEBUG_STREAM->end();} while(0)
#define DBGX(x) std::cerr << x << "\n"
#define DBGV(x) DBG("" #x " = " << x)

#define ERROR(x)  do {ERROR_STREAM->stream << x << "\n"; ERROR_STREAM->end();} while(0)
#include <algorithm>

template <class T> bool vector_remove(std::vector<T>& v, T val) {
	typename std::vector<T>::iterator position = std::find(v.begin(), v.end(), val);
	if (position != v.end()) {v.erase(position); return true;}
	return false;
}


class BasicException : public std::exception {
public:
	std::string s;
	BasicException(const char* w, const char* a) {s = w; s+= a;}
	virtual const char* what() const throw() { return s.c_str();}
};

template <class T> std::ostream& operator<<(std::ostream& os, std::vector<T*> v) {
	os << "[ ";
	for(uint i=0; i<v.size(); i++) {
		if(i!=0) os << ", ";
		os << typeid(v[i]).name() << ":" << v[i];
	}
	os << " ]";
	return os;
}

template <class T> std::ostream& operator<<(std::ostream& os, std::vector<T> v) {
	os << "[ ";
	for(uint i=0; i<v.size(); i++) {
		if(i!=0) os << ", ";
		os << v[i];
	}
	os << " ]";
	return os;
}

template <class T> std::ostream& operator<<(std::ostream& os, std::list<T> v) {
	os << "[ ";
	for(typename std::list<T>::iterator i=v.begin(); i!=v.end(); i++) {
		if(i!=v.begin()) os << ", ";
		os << *i;
	}
	os << " ]";
	return os;
}

template <typename T> void _VECTOR_ASSERT_UNICITY(std::vector<T> v, T item) {
	for(uint i=0; i<v.size(); i++) {
		if(v[i]==item) {
			throw "Not unique";
		}
	}
}

#ifdef DEBUG
#define VECTOR_ASSERT_UNICITY(vector, item) do {try {_VECTOR_ASSERT_UNICITY(vector, item); } catch(const char* s) {DBG("ERROR : not unique ! " << item << " in " << __FILE__ << ":" << __LINE__);}} while(0)
#else
#define VECTOR_ASSERT_UNICITY(vector, item)
#endif


inline void shell(const std::string& command) {(void) system(command.c_str());}


#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

void PRINT_STACK_TRACE();

std::string get_cur_thread_name();


void TIC();
void TAC();


#endif /* UTILS_H_ */
