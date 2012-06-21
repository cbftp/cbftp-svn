#pragma once

#include <string>
#include <vector>

#include "globalcontext.h"

extern GlobalContext * global;

class RawBuffer {
  private:
    std::vector<std::string> log;
    std::vector<std::string> logcopy;
    int latestp;
    int maxlength;
    std::string site;
    std::string id;
    bool inprogress;
    std::string getLine(int, bool);
  public:
    RawBuffer(int, std::string, std::string);
    void write(std::string);
    void writeLine(std::string);
    int getSize();
    std::string getLine(int);
    std::string getLineCopy(int);
    void freezeCopy();
};
