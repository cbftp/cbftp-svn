#pragma once

#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <map>
#include <cerrno>

#include "polling.h"
#include "semaphore.h"

namespace Core {

class PollingImpl : public PollingBase {
public:
  PollingImpl() :
    fds(new struct pollfd[MAX_EVENTS]), inwait(false), waiting(0) {
  }
  ~PollingImpl() {
    delete[] fds;
  }
  void wait(std::list<std::pair<int, PollEvent>>& fdlist) override {
    setInWait(true);
    fdlist.clear();
    while (!fdlist.size()) {
      awaitModifiers();
      int evcount = 0;
      maplock.lock();
      for (std::map<int, int>::const_iterator it = fdmap.begin();
           it != fdmap.end() && evcount < MAX_EVENTS; it++, evcount++)
      {
        fds[evcount].fd = it->first;
        fds[evcount].events = it->second;
      }
      maplock.unlock();
      if (poll(fds, evcount, 10)) {
        for (int i = 0; i < evcount; i++) {
          int fd = fds[i].fd;
          int revents = fds[i].revents;
          PollEvent pollevent = PollEvent::UNKNOWN;
          if (revents & POLLIN) {
            char c;
            if (recv(fd, &c, 1, MSG_PEEK) <= 0 && errno != ENOTSOCK) {
              removeFDIntern(fd);
            }
            pollevent = PollEvent::IN;
          }
          else if (revents & POLLOUT) {
            pollevent = PollEvent::OUT;
          }
          if (revents & (POLLHUP | POLLERR)) {
            removeFDIntern(fd);
          }
          if (pollevent != PollEvent::UNKNOWN) {
            fdlist.emplace_back(fd, pollevent);
          }
        }
      }
    }
    setInWait(false);

    awaitModifiers();
  }
  void addFDIn(int addfd) override {
    std::lock_guard<std::mutex> lock(maplock);
    std::map<int, int>::iterator it = fdmap.find(addfd);
    it != fdmap.end() ? it->second |= POLLIN : fdmap[addfd] = POLLIN;
  }
  void addFDOut(int addfd) override {
    std::lock_guard<std::mutex> lock(maplock);
    std::map<int, int>::iterator it = fdmap.find(addfd);
    it != fdmap.end() ? it->second |= POLLOUT : fdmap[addfd] = POLLOUT;
  }
  void removeFD(int delfd) override {
    awaitCurrentPollFinished();
    std::map<int, int>::iterator it = fdmap.find(delfd);
    if (it != fdmap.end()) {
      fdmap.erase(it);
    }
    maplock.unlock();
  }
  void setFDIn(int modfd) override {
    awaitCurrentPollFinished();
    fdmap[modfd] = POLLIN;
    maplock.unlock();
  }
  void setFDOut(int modfd) override {
    awaitCurrentPollFinished();
    fdmap[modfd] = POLLOUT;
    maplock.unlock();
  }
private:
  void awaitCurrentPollFinished() {
    waitlock.lock();
    if (inwait) {
      ++waiting;
      waitlock.unlock();
      waitsem.wait();
      maplock.lock();
      acksem.post();
      return;
    }
    maplock.lock();
    waitlock.unlock();
  }
  void awaitModifiers() {
    std::lock_guard<std::mutex> lock(waitlock);
    while (waiting) {
      waitsem.post();
      --waiting;
      acksem.wait();
    }
  }
  void setInWait(bool value) {
    std::lock_guard<std::mutex> lock(waitlock);
    inwait = value;
  }
  void removeFDIntern(int delfd) {
    fdmap.erase(delfd);
  }
  std::mutex maplock;
  struct pollfd* fds;
  std::map<int, int> fdmap;
  bool inwait;
  int waiting;
  std::mutex waitlock;
  Semaphore waitsem;
  Semaphore acksem;
};

} // namespace Core
