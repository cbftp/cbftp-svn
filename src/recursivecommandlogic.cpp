#include "recursivecommandlogic.h"

RecursiveCommandLogic::RecursiveCommandLogic() {

}

void RecursiveCommandLogic::initialize(int mode) {
  this->mode = mode;
  complete = false;
}

bool RecursiveCommandLogic::completed() {
  return complete;
}

std::string RecursiveCommandLogic::getRelevantPath() {
  return "";
}
