#pragma once

#include <sys/epoll.h>
#include <unistd.h>

#include "polling.h"

#define MAXEVENTS 32

class PollingImpl : public PollingBase {
public:
  PollingImpl() :
    epollfd(epoll_create(100)),
    events(new struct epoll_event[MAXEVENTS]) {
  }
  ~PollingImpl() {
    close(epollfd);
    delete[] events;
  }
  void wait(std::list<std::pair<int, PollEvent> > & fdlist) {
    fdlist.clear();
    int fds = epoll_wait(epollfd, events, MAXEVENTS, -1);
    for (int i = 0; i < fds; i++) {
      PollEvent pollevent = POLLEVENT_UNKNOWN;
      if (events[i].events & EPOLLIN) {
        pollevent = POLLEVENT_IN;
      }
      else if (events[i].events & EPOLLOUT) {
        pollevent = POLLEVENT_OUT;
      }
      fdlist.push_back(std::pair<int, PollEvent>(static_cast<int>(events[i].data.fd), pollevent));
    }
  }
  void addFDIn(int addfd) {
    control(EPOLLIN, EPOLL_CTL_ADD, addfd);
  }
  void addFDOut(int addfd) {
    control(EPOLLOUT, EPOLL_CTL_ADD, addfd);
  }
  void removeFD(int delfd) {
    control(EPOLLIN | EPOLLOUT, EPOLL_CTL_DEL, delfd);
  }
  void setFDIn(int modfd) {
    control(EPOLLIN, EPOLL_CTL_MOD, modfd);
  }
  void setFDOut(int modfd) {
    control(EPOLLOUT, EPOLL_CTL_MOD, modfd);
  }
private:
  void control(int ev, int op, int fd) {
    struct epoll_event event;
    event.events = ev;
    event.data.fd = fd;
    epoll_ctl(epollfd, op, fd, &event);
  }
  int epollfd;
  struct epoll_event * events;
};
