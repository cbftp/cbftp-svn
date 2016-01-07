#include "rawbuffer.h"

#include "uibase.h"
#include "globalcontext.h"
#include "util.h"

extern GlobalContext * global;

RawBuffer::RawBuffer(unsigned int maxlength, std::string site, std::string id) {
  latestp = 0;
  latestpcopy = 0;
  this->maxlength = maxlength;
  this->site = site;
  this->id = id;
  inprogress = false;
  uiwatching = false;
  threads = true;
  eventlog = false;
  writeLine("Log window initialized. Site: " + site + " Thread id: " + id);
}

RawBuffer::RawBuffer(std::string site) {
  latestp = 0;
  latestpcopy = 0;
  this->maxlength = 1024;
  this->site = site;
  inprogress = false;
  uiwatching = false;
  threads = false;
  eventlog = false;
  writeLine("Raw command window initialized. Site: " + site);
}

RawBuffer::RawBuffer() {
  latestp = 0;
  latestpcopy = 0;
  this->maxlength = 1024;
  inprogress = false;
  uiwatching = false;
  threads = false;
  eventlog = true;
  writeLine("Event log initialized.");
}
void RawBuffer::write(std::string s) {
  size_t split = s.find("\r\n");
  if (!split) {
    if (s.length() > 2) write(s.substr(2));
  }
  else if (split != std::string::npos) {
    write(s.substr(0, split));
    inprogress = false;
    if (s.length() > split + 2) write(s.substr(split + 2));
  }
  else {
    split = s.find("\n");
    if (!split) {
      if (s.length() > 1) write(s.substr(1));
    }
    else if (split != std::string::npos) {
      write(s.substr(0, split));
      inprogress = false;
      if (s.length() > split + 1) write(s.substr(split + 1));
    }
    else {
      if (inprogress) {
        log[(latestp > 0 ? latestp : maxlength) - 1].second.append(s);
      }
      else {
        std::string tag = "[" + util::ctimeLog() + (eventlog ? "" : " " + site + (threads ? " " + id : "")) + "]";
        if (log.size() < maxlength) {
          log.push_back(std::pair<std::string, std::string>(tag, s));
        }
        else {
          log[latestp] = std::pair<std::string, std::string>(tag, s);
        }
        if (++latestp == maxlength) {
          latestp = 0;
        }
        inprogress = true;
      }
    }
  }
  if (uiwatching) {
    global->getUIBase()->backendPush();
  }
}

void RawBuffer::writeLine(std::string s) {
  write(s + "\n");
}

void RawBuffer::rename(std::string name) {
  writeLine("Changing site name to: " + name);
  site = name;
}

void RawBuffer::setId(int id) {
  this->id = util::int2Str(id);
}

std::pair<std::string, std::string> RawBuffer::getLineCopy(unsigned int num) const {
  unsigned int size = getCopySize();
  if (num >= size) {
    return std::pair<std::string, std::string>("", "");
  }
  int pos = (num < latestpcopy ? latestpcopy - num - 1 : size + latestpcopy - num - 1);
  return logcopy[pos];
}

std::pair<std::string, std::string> RawBuffer::getLine(unsigned int num) const {
  unsigned int size = getSize();
  if (num >= size) {
    return std::pair<std::string, std::string>("", "");
  }
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

void RawBuffer::uiWatching(bool watching) {
  uiwatching = watching;
}
