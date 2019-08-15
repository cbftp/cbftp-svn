#pragma once

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>

#include "polling.h"

namespace Core {

class PollingImpl : public PollingBase {
public:
  PollingImpl() :
    kqueuefd(kqueue()),
    events(new struct kevent[MAX_EVENTS]) {
  }
  ~PollingImpl() {
    close(kqueuefd);
    delete[] events;
  }
  void wait(std::list<std::pair<int, PollEvent>>& fdlist) override {
    fdlist.clear();
    int fds = kevent(kqueuefd, nullptr, 0, events, MAX_EVENTS, nullptr);
    for (int i = 0; i < fds; i++) {
      PollEvent pollevent = PollEvent::UNKNOWN;
      if (events[i].filter == EVFILT_READ) {
        pollevent = PollEvent::IN;
      }
      else if (events[i].filter == EVFILT_WRITE) {
        pollevent = PollEvent::OUT;
      }
      fdlist.emplace_back(static_cast<int>(events[i].ident), pollevent);
    }
  }
  void addFDIn(int addfd) override {
    control(addfd, EVFILT_READ, EV_ADD);
  }
  void addFDOut(int addfd) override {
    control(addfd, EVFILT_WRITE, EV_ADD);
  }
  void removeFD(int delfd) override {
    control(delfd, EVFILT_READ | EVFILT_WRITE, EV_DELETE);
  }
  void setFDIn(int modfd) override {
    controlSet(modfd, EVFILT_READ, EVFILT_WRITE);
  }
  void setFDOut(int modfd) override {
    controlSet(modfd, EVFILT_WRITE, EVFILT_READ);
  }
private:
  void control(int fd, int filter, int flag) {
    struct kevent ev;
    EV_SET(&ev, fd, filter, flag, 0, 0, 0);
    kevent(kqueuefd, &ev, 1, nullptr, 0, nullptr);
  }
  void controlSet(int fd, int addfilter, int removefilter) {
    struct kevent ev[2];
    EV_SET(&ev[0], fd, removefilter, EV_DELETE, 0, 0, 0);
    EV_SET(&ev[1], fd, addfilter, EV_ADD, 0, 0, 0);
    kevent(kqueuefd, ev, 2, nullptr, 0, nullptr);
  }
  int kqueuefd;
  struct kevent* events;
};

} // namespace Core
