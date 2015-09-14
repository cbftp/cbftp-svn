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
  RawBuffer * getRawBuffer() const;
  int getLatestId() const;
  std::string getLatest() const;
  void log(std::string, std::string);
};
