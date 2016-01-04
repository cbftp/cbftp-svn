#pragma once

#include <string>

#include "pointer.h"

class EventReceiver;

class Event {
private:
  EventReceiver * receiver;
  Pointer<EventReceiver> preceiver;
  int type;
  char * data;
  int datalen;
  int numdata;
  std::string strdata;
public:
  Event(EventReceiver *, int, int, char *, int);
  Event(EventReceiver *, int);
  Event(EventReceiver *, int, int);
  Event(EventReceiver *, int, int, std::string);
  Event(Pointer<EventReceiver> &, int);
  ~Event();
  EventReceiver * getReceiver() const;
  int getType() const;
  char * getData() const;
  int getDataLen() const;
  int getNumericalData() const;
  std::string getStrData() const;
};
