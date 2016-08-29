#include "util.h"

#include <sstream>
#include <ctime>
#include <csignal>
#include <cctype>

namespace util {

int getSizeGranularity();
std::vector<unsigned long long int> getPowers();

static unsigned int sizegranularity = getSizeGranularity();
static std::vector<unsigned long long int> powers = getPowers();

std::string trim(std::string trimtarget) {
  int spos = 0;
  int epos = trimtarget.length() - 1;
  while (spos < epos && (trimtarget[spos] == ' ' || trimtarget[spos] == '\r' || trimtarget[spos] == '\n')) {
    ++spos;
  }
  while (epos >= spos && (trimtarget[epos] == ' ' || trimtarget[epos] == '\r' || trimtarget[epos] == '\n')) {
    --epos;
  }
  if (epos >= spos) {
    return trimtarget.substr(spos, epos - spos + 1);
  }
  return "";
}

int str2Int(std::string str) {
  int num;
  std::istringstream ss(str);
  ss >> num;
  return num;
}

std::string int2Str(unsigned int i) {
  return int2Str((int)i);
}

std::string int2Str(int i) {
  std::stringstream out;
  out << i;
  return out.str();
}

std::string int2Str(unsigned long long int i) {
  std::stringstream out;
  out << i;
  return out.str();
}

std::string simpleTimeFormat(int seconds) {
  std::string time;
  if (seconds >= 86400) {
    int days = seconds / 86400;
    time = int2Str(days) + "d";
    seconds = seconds % 86400;
  }
  if (seconds >= 3600) {
    int hours = seconds / 3600;
    time += int2Str(hours) + "h";
    seconds = seconds % 3600;
  }
  if (seconds >= 60) {
    int minutes = seconds / 60;
    time += int2Str(minutes) + "m";
    seconds = seconds % 60;
  }
  if (seconds || !time.length()) {
    time += int2Str(seconds) + "s";
  }
  return time;
}

std::string ctimeLog() {
  time_t rawtime = time(NULL);
  char timebuf[26];
  ctime_r(&rawtime, timebuf);
  return std::string(timebuf + 11, 8);
}

std::string & debugString(const char * s) {
    return *(new std::string(s));
}

std::string parseSize(unsigned long long int size) {
  int iprefix;
  for (iprefix = 0; iprefix < 6 && size / powers[iprefix] >= 1000; iprefix++);
  unsigned long long int currentpower = powers[iprefix];
  std::string result;
  int whole = size / currentpower;
  if (iprefix == 0) {
    result = int2Str(whole) + " B";
  }
  else {
    unsigned long long int decim = ((size % currentpower) * sizegranularity) / currentpower + 5;
    if (decim >= sizegranularity) {
      whole++;
      decim = 0;
    }
    std::string decimstr = int2Str(decim);
    while (decimstr.length() <= SIZEDECIMALS) {
      decimstr = "0" + decimstr;
    }
    result = int2Str(whole) + "." + decimstr.substr(0, SIZEDECIMALS) + " ";
    switch (iprefix) {
      case 1:
        result.append("kB");
        break;
      case 2:
        result.append("MB");
        break;
      case 3:
        result.append("GB");
        break;
      case 4:
        result.append("TB");
        break;
      case 5:
        result.append("PB");
        break;
      case 6:
        result.append("EB");
        break;
    }
  }
  return result;
}

int getSizeGranularity() {
  int gran = 1;
  for (int i = 0; i <= SIZEDECIMALS; i++) {
    gran *= 10;
  }
  return gran;
}

std::vector<unsigned long long int> getPowers() {
  std::vector<unsigned long long int> vec;
  vec.reserve(7);
  unsigned long long int pow = 1;
  for (int i = 0; i < 7; i++) {
    vec.push_back(pow);
    pow *= 1024;
  }
  return vec;
}

std::string getGroupNameFromRelease(std::string release) {
  size_t splitpos = release.rfind("-");
  if (splitpos != std::string::npos) {
    return release.substr(splitpos + 1);
  }
  else {
    return "";
  }
}

void assert(bool condition) {
  if (!condition) {
    raise(SIGTRAP);
  }
}

std::string toLower(const std::string & in) {
  std::string ret = in;
  for (unsigned int i = 0; i < in.length(); i++) {
    ret[i] = tolower(ret[i]);
  }
  return ret;
}

}
