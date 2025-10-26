#pragma once

#include <string>
#include <vector>
#include <list>
#include <regex>
#include <set>
#include <unordered_map>
#include <unordered_set>

#define SIZEPOWER 1024
#define SIZEDECIMALS 2

namespace util {

struct Result {
  Result();
  Result(bool success, const std::string& error = "");
  bool success;
  std::string error;
};

std::string trim(const std::string &);
std::list<std::string> trim(const std::list<std::string> & in);
std::vector<std::string> trim(const std::vector<std::string> & in);
std::string simpleTimeFormat(int);
std::string & debugString(const char *);
std::string parseSize(unsigned long long int);
std::string getGroupNameFromRelease(const std::string &);
std::string toLower(const std::string &);
bool isHex(const std::string& hex);
std::string urlDecode(const std::string& url);
int wildcmp(const char *, const char *);
int wildcmpCase(const char *, const char *);
std::list<std::string> split(const std::string& in, const std::string& sep = " ");
std::vector<std::string> splitVec(const std::string& in, const std::string& sep = " ");
std::string join(const std::list<std::string>& in, const std::string& sep = " ");
std::string join(const std::vector<std::string>& in, const std::string& sep = " ");
std::string join(const std::set<std::string>& in, const std::string& sep = " ");
std::unordered_set<std::string> merge(const std::unordered_set<std::string>& in1, const std::unordered_set<std::string>& in2);
std::unordered_set<std::string> merge(const std::unordered_map<std::string, unsigned long long int>& in1, const std::unordered_set<std::string>& in2);
int chrstrfind(const char *, unsigned int, const char *, unsigned int);
int chrfind(const char *, unsigned int, char);
bool eightCharUserCompare(const std::string & a, const std::string & b);
std::regex regexParse(const std::string & pattern);
struct naturalComparator {
  bool operator()(const std::string& a, const std::string& b) const;
};
}
