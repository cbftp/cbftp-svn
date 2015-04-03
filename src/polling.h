#pragma once

#include <utility>
#include <list>

enum PollEvent {
  POLLEVENT_IN,
  POLLEVENT_OUT,
  POLLEVENT_UNKNOWN
};

class Polling {
public:
  virtual ~Polling() {
  }
  virtual void wait(std::list<std::pair<int, PollEvent> > &) = 0;
  virtual void addFDIn(int) = 0;
  virtual void addFDOut(int) = 0;
  virtual void removeFDIn(int) = 0;
  virtual void removeFDOut(int) = 0;
  virtual void setFDIn(int) = 0;
  virtual void setFDOut(int) = 0;
};
