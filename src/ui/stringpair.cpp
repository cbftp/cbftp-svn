#include "stringpair.h"

StringPair::StringPair(std::string key, std::string value) {
  this->key = key;
  this->value = value;
}

std::string StringPair::getKey() {
  return key;
}

std::string StringPair::getValue() {
  return value;
}
