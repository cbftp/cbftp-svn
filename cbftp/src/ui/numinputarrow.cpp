#include "numinputarrow.h"

NumInputArrow::NumInputArrow() {

}

NumInputArrow::NumInputArrow(int startval, int min, int max) {
  val = startval;
  active = false;
  this->min = min;
  this->max = max;
}

bool NumInputArrow::decrease() {
  if (val > min) {
    val--;
    return true;
  }
  return false;

}

bool NumInputArrow::increase() {
  if (val < max) {
    val++;
    return true;
  }
  return false;
}

bool NumInputArrow::setValue(int newval) {
  if (newval >= min && newval <= max) {
    val = newval;
    return true;
  }
  return false;
}

int NumInputArrow::getValue() const {
  return val;
}

void NumInputArrow::activate() {
  active = true;
}

void NumInputArrow::deactivate() {
  active = false;
}
std::string NumInputArrow::getVisual() const {
  int maxlen = std::to_string(max).length() + 4;
  std::string out = "";
  if (active) {
    out = "< " + std::to_string(val) + " >";
  }
  else {
    out = std::to_string(val);
  }
  while ((int) out.length() < maxlen) out += ' ';
  return out;
}
