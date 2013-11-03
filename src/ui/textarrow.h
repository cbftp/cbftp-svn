#pragma once

#include <string>
#include <vector>

class TextArrow {
public:
  TextArrow();
  int getOption();
  std::string getOptionText();
  bool setOption(int);
  bool setOptionText(std::string);
  bool next();
  bool previous();
  std::string getVisual();
  void activate();
  void deactivate();
  bool isActive();
  void addOption(std::string, int);
  void clear();
private:
  bool active;
  std::vector<std::pair<int, std::string> > options;
  unsigned int currentpos;
  unsigned int maxlen;
};
