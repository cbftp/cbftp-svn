#pragma once

#include <string>
#include <vector>

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
    bool eventlog;
  public:
    RawBuffer(unsigned int, std::string, std::string);
    RawBuffer(std::string);
    RawBuffer();
    void setId(int);
    void write(std::string);
    void writeLine(std::string);
    void rename(std::string);
    unsigned int getSize() const;
    unsigned int getCopySize() const;
    std::string getLine(unsigned int) const;
    std::string getLineCopy(unsigned int) const;
    void freezeCopy();
    void uiWatching(bool);
};
