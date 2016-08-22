#pragma once

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>

#include "polling.h"

#define MAXEVENTS 32

class PollingImpl : public PollingBase {
public:
  PollingImpl() :
    kqueuefd(kqueue()),
    events(new struct kevent[MAXEVENTS]) {
  }
  ~PollingImpl() {
    close(kqueuefd);
    delete[] events;
  }
  void wait(std::list<std::pair<int, PollEvent> > & fdlist) {
    fdlist.clear();
    int fds = kevent(kqueuefd, NULL, 0, events, MAXEVENTS, NULL);
    for (int i = 0; i < fds; i++) {
      PollEvent pollevent = POLLEVENT_UNKNOWN;
      if (events[i].filter == EVFILT_READ) {
        pollevent = POLLEVENT_IN;
      }
      else if (events[i].filter == EVFILT_WRITE) {
        pollevent = POLLEVENT_OUT;
      }
      fdlist.push_back(std::pair<int, PollEvent>(static_cast<int>(events[i].ident), pollevent));
    }
  }
  void addFDIn(int addfd) {
    control(addfd, EVFILT_READ, EV_ADD);
  }
  void addFDOut(int addfd) {
    control(addfd, EVFILT_WRITE, EV_ADD);
  }
  void removeFD(int delfd) {
    control(delfd, EVFILT_READ | EVFILT_WRITE, EV_DELETE);
  }
  void setFDIn(int modfd) {
    controlSet(modfd, EVFILT_READ, EVFILT_WRITE);
  }
  void setFDOut(int modfd) {
    controlSet(modfd, EVFILT_WRITE, EVFILT_READ);
  }
private:
  void control(int fd, int filter, int flag) {
    struct kevent ev;
    EV_SET(&ev, fd, filter, flag, 0, 0, 0);
    kevent(kqueuefd, &ev, 1, NULL, 0, NULL);
  }
  void controlSet(int fd, int addfilter, int removefilter) {
    struct kevent ev[2];
    EV_SET(&ev[0], fd, removefilter, EV_DELETE, 0, 0, 0);
    EV_SET(&ev[1], fd, addfilter, EV_ADD, 0, 0, 0);
    kevent(kqueuefd, ev, 2, NULL, 0, NULL);
  }
  int kqueuefd;
  struct kevent * events;
};
