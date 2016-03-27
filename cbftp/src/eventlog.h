#pragma once

#include <string>

#include "core/logger.h"

class RawBuffer;

class EventLog : public Logger {
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
  void log(const std::string &, const std::string &);
};
