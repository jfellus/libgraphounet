/*
 * file.h
 *
 *  Created on: 1 oct. 2014
 *      Author: jfellus
 */

#ifndef FILE_H_
#define FILE_H_

#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include "utils.h"

bool file_has_ext(const char* filename, const char* ext);
inline bool file_has_ext(const std::string& filename, const char* ext) {  return file_has_ext(filename.c_str(), ext); }

inline std::string file_change_ext(const std::string& filename, const char* newext) {
	uint i = filename.rfind('.');
	if(i==std::string::npos) return filename + newext;
	return filename.substr(0, i) + newext;
}

void set_argv_0(const char* argv0);

inline bool file_is_absolute(const std::string& file) {
	return file[0]=='/';
}

std::string file_basename(const std::string& file);
std::string file_dirname(const std::string& file);

bool file_exists(const std::string& filename);
bool file_is_directory(const std::string& filename);
std::string file_absolute_path(const std::string& path);

void fcomeBackToLine(std::istream& f, int linesBefore);
std::string fgetlines(std::istream& f, int nblines);

void f_read_lines(const std::string& filename, std::vector<std::string>& lines);
void f_write_lines(const std::string& filename, const std::vector<std::string>& lines);
void f_write_line(const std::string& filename, const std::string& s);


std::string f_read_comments(std::istream& f);
void f_write_comments(std::ostream& f, const std::string& comments);

template <class T> bool f_try_read2(std::istream& f, const char* fmt, T& val) {
	int i;
	char c;
	for(i=0; fmt[i]!=0; i++) {
		if(fmt[i]=='%') {
			while((c = f.get())==' ' || c=='\t');
			if(c=='\n') throw(fmt);
			f.unget();
			i++;
			f >> val;
		} else {
			c = f.get();
			if(c!=fmt[i]) {f.unget(); throw(fmt);}
			while((fmt[i]==' ' || fmt[i]=='\t') && (fmt[i+1]==' ' || fmt[i+1]=='\t')) i++;
		}
	}
	return true;
}

template <class T> bool f_try_read(std::istream& f, const char* fmt, T& val) {
	std::istringstream in(fmt);
	std::string token; std::string s;
	while(in.good()) {
		in >> token;
		if(token[0]=='%') f >> val;
		else {
			f >> s;
			if(s!=token) {
				for(uint i=0; i<s.length(); i++) f.unget();
				throw token;
			}
		}
	}
	return true;
}

void f_skipline(std::ifstream& f);

std::string cwd();
std::string home();
std::string main_dir();

void create_dir_for(const std::string& filename);

long file_get_modification_time(const std::string& filename);


#endif /* FILE_H_ */
