#include "rawbuffer.h"

#include <cassert>

#include "uibase.h"
#include "globalcontext.h"
#include "util.h"
#include "rawbuffercallback.h"

RawBuffer::RawBuffer(unsigned int maxlength, std::string site, std::string id) :
  latestp(0),
  latestpcopy(0),
  maxlength(maxlength),
  bookmarklines(0),
  site(site),
  id(id),
  inprogress(false),
  uiwatching(false),
  threads(true),
  eventlog(false),
  callback(NULL)
{
  writeLine("Log window initialized. Site: " + site + " Thread id: " + id);
}

RawBuffer::RawBuffer(std::string site) :
  latestp(0),
  latestpcopy(0),
  maxlength(1024),
  bookmarklines(0),
  site(site),
  inprogress(false),
  uiwatching(false),
  threads(false),
  eventlog(false),
  callback(NULL)
{
  writeLine("Raw command window initialized. Site: " + site);
}

RawBuffer::RawBuffer() :
  latestp(0),
  latestpcopy(0),
  maxlength(1024),
  bookmarklines(0),
  inprogress(false),
  uiwatching(false),
  threads(false),
  eventlog(true),
  callback(NULL)
{
  writeLine("Event log initialized.");
}

void RawBuffer::setCallback(RawBufferCallback * callback) {
  this->callback = callback;
}

void RawBuffer::unsetCallback() {
  callback = NULL;
}

void RawBuffer::bookmark() {
  bookmarklines = 0;
}

unsigned int RawBuffer::linesSinceBookmark() const {
  return bookmarklines;
}

void RawBuffer::lineFinished() {
  inprogress = false;
  if (callback != NULL) {
    callback->newRawBufferLine(getLine(0));
  }
}
void RawBuffer::write(const std::string & s) {
  write(getTag(), s);
}

void RawBuffer::write(const std::string & tag, const std::string & s) {
  size_t pos = 0;
  size_t len = s.length();
  while (pos < len) {
    size_t split = s.find('\n', pos);
    if (split == pos) {
      lineFinished();
      ++pos;
    }
    else if (split == std::string::npos) {
      addLogText(tag, s.substr(pos));
      pos = len;
    }
    else {
      size_t len = split - pos;
      if (s[split - 1] == '\r') {
        --len;
      }
      addLogText(tag, s.substr(pos, len));
      pos = split;
    }
  }
  if (uiwatching) {
    global->getUIBase()->backendPush();
  }
}

void RawBuffer::addLogText(const std::string& tag, const std::string& text) {
  if (inprogress) {
    log[(latestp > 0 ? latestp : maxlength) - 1].second.append(text);
    return;
  }
  if (log.size() < maxlength) {
    log.emplace_back(tag, text);
  }
  else {
    log[latestp] = std::make_pair(tag, text);
  }
  if (++latestp == maxlength) {
    latestp = 0;
  }
  ++bookmarklines;
  inprogress = true;
}

void RawBuffer::writeLine(const std::string & s) {
  writeLine(getTag(), s);
}

void RawBuffer::writeLine(const std::string & tag, const std::string & s) {
  write(tag, s + "\n");
}

void RawBuffer::rename(std::string name) {
  writeLine("Changing site name to: " + name);
  site = name;
}

std::string RawBuffer::getTag() const {
  return "[" + util::ctimeLog() + (eventlog ? "" : " " + site + (threads ? " " + id : "")) + "]";
}

void RawBuffer::setId(int id) {
  this->id = std::to_string(id);
}

const std::pair<std::string, std::string> & RawBuffer::getLineCopy(unsigned int num) const {
  unsigned int size = getCopySize();
  assert(num < size);
  int pos = (num < latestpcopy ? latestpcopy - num - 1 : size + latestpcopy - num - 1);
  return logcopy[pos];
}

const std::pair<std::string, std::string> & RawBuffer::getLine(unsigned int num) const {
  unsigned int size = getSize();
  assert(num < size);
  int pos = (num < latestp ? latestp - num - 1 : size + latestp - num - 1);
  return log[pos];
}

unsigned int RawBuffer::getSize() const {
  return log.size();
}

unsigned int RawBuffer::getCopySize() const {
  return logcopy.size();
}

void RawBuffer::freezeCopy() {
  logcopy = log;
  latestpcopy = latestp;
}

void RawBuffer::setUiWatching(bool watching) {
  uiwatching = watching;
}

void RawBuffer::clear() {
  log.clear();
  logcopy.clear();
  latestp = 0;
  latestpcopy = 0;
  inprogress = false;
}
