#pragma once

#include <string>
#include <vector>

class GlobalContext;

extern GlobalContext * global;

class RawBuffer {
  private:
    std::vector<std::string> log;
    std::vector<std::string> logcopy;
    unsigned int latestp;
    unsigned int latestpcopy;
    unsigned int maxlength;
    std::string site;
    std::string id;
    bool inprogress;
    bool threads;
    bool uiwatching;
  public:
    RawBuffer(unsigned int, std::string, std::string);
    RawBuffer(std::string);
    void setId(int);
    void write(std::string);
    void writeLine(std::string);
    void rename(std::string);
    unsigned int getSize();
    unsigned int getCopySize();
    std::string getLine(unsigned int);
    std::string getLineCopy(unsigned int);
    void freezeCopy();
    void uiWatching(bool);
};
