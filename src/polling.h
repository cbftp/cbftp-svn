#pragma once

#include <utility>
#include <list>

enum PollEvent {
  POLLEVENT_IN,
  POLLEVENT_OUT,
  POLLEVENT_UNKNOWN
};

class PollingBase {
public:
  virtual ~PollingBase() {
  }
  virtual void wait(std::list<std::pair<int, PollEvent> > &) = 0;
  virtual void addFDIn(int) = 0;
  virtual void addFDOut(int) = 0;
  virtual void removeFDIn(int) = 0;
  virtual void removeFDOut(int) = 0;
  virtual void setFDIn(int) = 0;
  virtual void setFDOut(int) = 0;
};

#ifdef __linux
#include "pollingpoll.h"
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) ||\
    defined(__APPLE__)
#include "pollingkqueue.h"
#else
#include "pollingpoll.h"
#endif

class Polling {
public:
  void wait(std::list<std::pair<int, PollEvent> > & events) {
    impl.wait(events);
  }
  void addFDIn(int addfd) {
    impl.addFDIn(addfd);
  }
  void addFDOut(int addfd) {
    impl.addFDOut(addfd);
  }
  void removeFDIn(int delfd) {
    impl.removeFDIn(delfd);
  }
  void removeFDOut(int delfd) {
    impl.removeFDOut(delfd);
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
