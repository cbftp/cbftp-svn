#pragma once

#include <string>
#include <vector>
#include <list>

#define SIZEPOWER 1024
#define SIZEDECIMALS 2

namespace util {

std::string trim(const std::string &);
unsigned long long int str2Int(const std::string &);
std::string int2Str(int);
std::string int2Str(unsigned int);
std::string int2Str(unsigned long long int);
std::string simpleTimeFormat(int);
std::string ctimeLog();
std::string & debugString(const char *);
std::string parseSize(unsigned long long int);
std::string getGroupNameFromRelease(const std::string &);
void assert(bool);
std::string toLower(const std::string &);
int wildcmp(const char *, const char *);
int wildcmpCase(const char *, const char *);
std::list<std::string> split(const std::string &, const std::string &);
std::list<std::string> split(const std::string &);
std::vector<std::string> splitVec(const std::string &, const std::string &);
std::vector<std::string> splitVec(const std::string &);
std::string join(const std::list<std::string> &, const std::string &);
std::string join(const std::list<std::string> &);
std::string join(const std::vector<std::string> &, const std::string &);
std::string join(const std::vector<std::string> &);
int chrstrfind(const char *, unsigned int, const char *, unsigned int);
int chrfind(const char *, unsigned int, char);
}
