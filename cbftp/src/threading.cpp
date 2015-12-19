#include "threading.h"

namespace Threading {

void setThreadName(pthread_t thread, const char * name) {
#ifdef _ISOC95_SOURCE
  pthread_setname_np(thread, name);
#endif
}

void setCurrentThreadName(const char * name) {
  setThreadName(pthread_self(), name);
}

}
