#include "eventlog.h"

#include "rawbuffer.h"

EventLog::EventLog() {
  rawbuf = new RawBuffer();
  latestid = 0;
}

EventLog::~EventLog() {
  delete rawbuf;
}

RawBuffer * EventLog::getRawBuffer() {
  return rawbuf;
}

void EventLog::log(std::string owner, std::string text) {
  std::string line = "<" + owner + "> " + text;
  latest = line;
  latestid++;
  rawbuf->writeLine("<" + owner + "> " + text);
}

std::string EventLog::getLatest() {
  return latest;
}

int EventLog::getLatestId() {
  return latestid;
}
