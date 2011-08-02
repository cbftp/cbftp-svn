#pragma once
#include <string>
#include <vector>

class RawBuffer {
  private:
  std::vector<std::string> log;
  int latestp;
  int maxlength;
  bool inprogress;
  public:
    RawBuffer(int, std::string, std::string);
    void write(std::string);
    void writeLine(std::string);
};
