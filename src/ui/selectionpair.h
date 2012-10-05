#pragma once

#include <string>

class SelectionPair {
  private:
    std::string path;
    std::string selection;
  public:
    SelectionPair(std::string, std::string);
    std::string getPath();
    std::string getSelection();
};
