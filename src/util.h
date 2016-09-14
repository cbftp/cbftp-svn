#pragma once

#include <string>
#include <vector>

#define SIZEPOWER 1024
#define SIZEDECIMALS 2

namespace util {

std::string trim(std::string);
int str2Int(std::string);
std::string int2Str(int);
std::string int2Str(unsigned int);
std::string int2Str(unsigned long long int);
std::string simpleTimeFormat(int);
std::string ctimeLog();
std::string & debugString(const char *);
std::string parseSize(unsigned long long int);
std::string getGroupNameFromRelease(std::string);
void assert(bool);
std::string toLower(const std::string &);
std::string cleanPath(const std::string &);
int wildcmp(const char *, const char *);
int wildcmpCase(const char *, const char *);
}
