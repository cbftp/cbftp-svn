#pragma once

#include <string>

class NumInputArrow {
public:
  NumInputArrow();
  NumInputArrow(int, int, int);
  int getValue();
  bool setValue(int);
  bool increase();
  bool decrease();
  std::string getVisual();
  void activate();
  void deactivate();
private:
  int val;
  int min;
  int max;
  bool active;
};
