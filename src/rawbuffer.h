#pragma once

#include <string>
#include <vector>

#include "globalcontext.h"
#include "ui/uicommunicator.h"

extern GlobalContext * global;

class RawBuffer {
  private:
    std::vector<std::string> log;
    std::vector<std::string> logcopy;
    unsigned int latestp;
    unsigned int maxlength;
    std::string site;
    std::string id;
    bool inprogress;
    bool uiwatching;
    std::string getLine(unsigned int, bool);
  public:
    RawBuffer(unsigned int, std::string, std::string);
    void setId(int);
    void write(std::string);
    void writeLine(std::string);
    unsigned int getSize();
    std::string getLine(unsigned int);
    std::string getLineCopy(unsigned int);
    void freezeCopy();
    void uiWatching(bool);
};
