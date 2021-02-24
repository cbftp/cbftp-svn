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
  void wait(std::list<PollEvent>& eventlist) override {
    setInWait(true);
    eventlist.clear();
    while (!eventlist.size()) {
      awaitModifiers();
      int evcount = 0;
      maplock.lock();
      for (std::map<int, Event>::const_iterator it = fdmap.begin();
           it != fdmap.end() && evcount < MAX_EVENTS; it++, evcount++)
      {
        fds[evcount].fd = it->first;
        fds[evcount].events = it->second.events;
      }
      maplock.unlock();
      if (poll(fds, evcount, 10)) {
        for (int i = 0; i < evcount; i++) {
          int fd = fds[i].fd;
          int revents = fds[i].revents;
          PollEventType type = PollEventType::UNKNOWN;
          if (revents & POLLIN) {
            char c;
            if (recv(fd, &c, 1, MSG_PEEK) <= 0 && errno != ENOTSOCK) {
              removeFDIntern(fd);
            }
            type = PollEventType::IN;
          }
          else if (revents & POLLOUT) {
            type = PollEventType::OUT;
          }
          if (revents & (POLLHUP | POLLERR)) {
            removeFDIntern(fd);
          }
          if (type != PollEventType::UNKNOWN) {
            maplock.lock();
            std::map<int, Event>::const_iterator it = fdmap.find(fd);
            unsigned int userdata = it != fdmap.end() ? it->second.userdata : 0;
            maplock.unlock();
            eventlist.emplace_back(fd, type, userdata);
          }
        }
      }
    }
    setInWait(false);

    awaitModifiers();
  }
  void addFDIn(int addfd, unsigned int userdata) override {
    std::lock_guard<std::mutex> lock(maplock);
    std::map<int, Event>::iterator it = fdmap.find(addfd);
    if (it != fdmap.end()) {
      it->second.events |= POLLIN;
    }
    else {
      fdmap[addfd] = {POLLIN, userdata};
    }
  }
  void addFDOut(int addfd, unsigned int userdata) override {
    std::lock_guard<std::mutex> lock(maplock);
    std::map<int, Event>::iterator it = fdmap.find(addfd);
    if (it != fdmap.end()) {
      it->second.events |= POLLOUT;
    }
    else {
      fdmap[addfd] = {POLLOUT, userdata};
    }
  }
  void removeFD(int delfd) override {
    awaitCurrentPollFinished();
    std::map<int, Event>::iterator it = fdmap.find(delfd);
    if (it != fdmap.end()) {
      fdmap.erase(it);
    }
    maplock.unlock();
  }
  void setFDIn(int modfd, int unsigned userdata) override {
    awaitCurrentPollFinished();
    fdmap[modfd] = {POLLIN, userdata};
    maplock.unlock();
  }
  void setFDOut(int modfd, unsigned int userdata) override {
    awaitCurrentPollFinished();
    fdmap[modfd] = {POLLOUT, userdata};
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
  struct Event {
    int events;
    unsigned int userdata;
  };
  std::map<int, Event> fdmap;
  bool inwait;
  int waiting;
  std::mutex waitlock;
  Semaphore waitsem;
  Semaphore acksem;
};

} // namespace Core
