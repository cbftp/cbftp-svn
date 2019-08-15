#pragma once

#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>

#include "polling.h"

namespace Core {

class PollingImpl : public PollingBase {
public:
  PollingImpl() :
    epollfd(epoll_create(100)),
    events(new struct epoll_event[MAX_EVENTS]) {
  }
  ~PollingImpl() {
    close(epollfd);
    delete[] events;
  }
  void wait(std::list<std::pair<int, PollEvent>>& fdlist) override {
    fdlist.clear();
    int fds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
    for (int i = 0; i < fds; i++) {
      PollEvent pollevent = PollEvent::UNKNOWN;
      if (events[i].events & EPOLLIN) {
        pollevent = PollEvent::IN;
      }
      else if (events[i].events & EPOLLOUT) {
        pollevent = PollEvent::OUT;
      }
      fdlist.emplace_back(static_cast<int>(events[i].data.fd), pollevent);
    }
  }
  void addFDIn(int addfd) override {
    control(EPOLLIN, EPOLL_CTL_ADD, addfd);
  }
  void addFDOut(int addfd) override {
    control(EPOLLOUT, EPOLL_CTL_ADD, addfd);
  }
  void removeFD(int delfd) override {
    control(EPOLLIN | EPOLLOUT, EPOLL_CTL_DEL, delfd);
  }
  void setFDIn(int modfd) override {
    control(EPOLLIN, EPOLL_CTL_MOD, modfd);
  }
  void setFDOut(int modfd) override {
    control(EPOLLOUT, EPOLL_CTL_MOD, modfd);
  }
private:
  void control(int ev, int op, int fd) {
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = ev;
    event.data.fd = fd;
    epoll_ctl(epollfd, op, fd, &event);
  }
  int epollfd;
  struct epoll_event* events;
};

} // namespace Core
