#include "rawbuffer.h"

RawBuffer::RawBuffer(int maxlength, std::string site, std::string id) {
  log.push_back("Log window initialized. Site: " + site + " Thread id: " + id);
  this->maxlength = maxlength;
  latestp = 0;
  inprogress = false;
}

void RawBuffer::write(std::string s) {
  int split = s.find("\n");
  if (split >= 0) {
    write(s.substr(0, split));
    inprogress = false;
    if (s.length() > split + 1) write(s.substr(split + 1));
  }
  else {
    if (inprogress) log[latestp].append(s);
    else {
      if (log.size() < maxlength) log.push_back(s);
      else log[latestp] = s;
    }
    if (++latestp == maxlength) latestp = 0;
    inprogress = true;
  }
}

void RawBuffer::writeLine(std::string s) {
  write(s + "\n");
}
