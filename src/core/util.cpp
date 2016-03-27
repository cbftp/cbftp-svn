#include "util.h"

#include <signal.h>
#include <sstream>

namespace coreutil {

std::string int2Str(int i) {
  std::stringstream out;
  out << i;
  return out.str();
}

void assert(bool condition) {
  if (!condition) {
    raise(SIGTRAP);
  }
}

}
