#include "selectionpair.h"

SelectionPair::SelectionPair(std::string path, std::string selection) {
  this->path = path;
  this->selection = selection;
}

std::string SelectionPair::getPath() {
  return path;
}

std::string SelectionPair::getSelection() {
  return selection;
}
