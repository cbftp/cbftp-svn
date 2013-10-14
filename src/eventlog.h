#pragma once

#include <string>

class RawBuffer;

class EventLog {
private:
  RawBuffer * rawbuf;
public:
  EventLog();
  RawBuffer * getRawBuffer();
  void log(std::string, std::string);
};
