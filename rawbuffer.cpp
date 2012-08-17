#include "rawbuffer.h"
#include <iostream>
RawBuffer::RawBuffer(int maxlength, std::string site, std::string id) {
  latestp = 0;
  this->maxlength = maxlength;
  this->site = site;
  this->id = id;
  inprogress = false;
  uiwatching = false;
  writeLine("Log window initialized. Site: " + site + " Thread id: " + id);
}

void RawBuffer::write(std::string s) {
  unsigned int split = s.find("\n");
  if (!split) {
    if (s.length() > 1) write(s.substr(1));
  }
  else if (split > 0) {
    write(s.substr(0, split));
    inprogress = false;
    if (s.length() > split + 1) write(s.substr(split + 1));
  }
  else {
    if (inprogress) {
      log[(latestp > 0 ? latestp : maxlength) - 1].append(s);
    }
    else {
      s = "[" + global->ctimeLog() + " " + site + " " + id + "] " + s;
      if ((int) log.size() < maxlength) log.push_back(s);
      else log[latestp] = s;
      if (++latestp == maxlength) latestp = 0;
      inprogress = true;
    }
  }
  if (uiwatching) {
    global->getUICommunicator()->backendPush();
  }
}

void RawBuffer::writeLine(std::string s) {
  write(s + "\n");
}

std::string RawBuffer::getLineCopy(int num) {
  return getLine(num, true);
}

std::string RawBuffer::getLine(int num) {
  return getLine(num, false);
}

std::string RawBuffer::getLine(int num, bool fromcopy) {
  int size = getSize();
  if (num >= size || num < 0) return "";
  int pos = (num < latestp ? latestp - num - 1 : size - latestp - num - 1);
  return fromcopy ? logcopy[pos] : log[pos];
}

int RawBuffer::getSize() {
  return log.size();
}

void RawBuffer::freezeCopy() {
  logcopy = log;
}

void RawBuffer::uiWatching(bool watching) {
  uiwatching = watching;
}
