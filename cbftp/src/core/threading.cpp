#include "threading.h"

#include <cassert>

namespace Core {
namespace Threading {

void setThreadName(pthread_t thread, const std::string& name) {
  assert(name.length() <= 15);
#ifdef _ISOC95_SOURCE
  pthread_setname_np(thread, name.c_str());
#endif
}

void setCurrentThreadName(const std::string& name) {
  assert(name.length() <= 15);
  setThreadName(pthread_self(), name);
}

}
}
