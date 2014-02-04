#pragma once

#include <string>

class RawBuffer;

class EventLog {
private:
  RawBuffer * rawbuf;
  std::string latest;
  int latestid;
public:
  EventLog();
  ~EventLog();
  RawBuffer * getRawBuffer();
  int getLatestId();
  std::string getLatest();
  void log(std::string, std::string);
};
