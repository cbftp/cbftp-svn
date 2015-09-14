#pragma once

#include <string>

class NumInputArrow {
public:
  NumInputArrow();
  NumInputArrow(int, int, int);
  int getValue() const;
  bool setValue(int);
  bool increase();
  bool decrease();
  std::string getVisual() const;
  void activate();
  void deactivate();
private:
  int val;
  int min;
  int max;
  bool active;
};
