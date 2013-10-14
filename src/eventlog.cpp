#include "eventlog.h"

#include "rawbuffer.h"

EventLog::EventLog() {
  rawbuf = new RawBuffer();
}

RawBuffer * EventLog::getRawBuffer() {
  return rawbuf;
}

void EventLog::log(std::string owner, std::string text) {
  rawbuf->writeLine("<" + owner + "> " + text);
}
