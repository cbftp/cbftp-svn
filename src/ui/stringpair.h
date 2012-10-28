#pragma once

#include <string>

class StringPair {
  private:
    std::string key;
    std::string value;
  public:
    StringPair(std::string, std::string);
    std::string getKey();
    std::string getValue();
};
