#pragma once

#include <list>
#include <utility>

namespace Core {

#define MAX_EVENTS 256

enum class PollEvent {
  IN,
  OUT,
  UNKNOWN
};

class PollingBase {
public:
  virtual ~PollingBase() {
  }
  virtual void wait(std::list<std::pair<int, PollEvent>>& events) = 0;
  virtual void addFDIn(int) = 0;
  virtual void addFDOut(int) = 0;
  virtual void removeFD(int) = 0;
  virtual void setFDIn(int) = 0;
  virtual void setFDOut(int) = 0;
};

} // namespace Core

#ifdef __linux
#include "pollingepoll.h"
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) ||\
    defined(__APPLE__)
#include "pollingkqueue.h"
#else
#include "pollingpoll.h"
#endif

namespace Core {

class Polling {
public:
  void wait(std::list<std::pair<int, PollEvent>>& events) {
    impl.wait(events);
  }
  void addFDIn(int addfd) {
    impl.addFDIn(addfd);
  }
  void addFDOut(int addfd) {
    impl.addFDOut(addfd);
  }
  void removeFD(int delfd) {
    impl.removeFD(delfd);
  }
  void setFDIn(int modfd) {
    impl.setFDIn(modfd);
  }
  void setFDOut(int modfd) {
    impl.setFDOut(modfd);
  }
private:
  PollingImpl impl;
};

} // namespace Core
