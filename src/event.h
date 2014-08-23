#pragma once

#include <string>

class EventReceiver;

class Event {
private:
  EventReceiver * receiver;
  int type;
  char * data;
  int datalen;
  int interval;
  std::string strdata;
public:
  Event(EventReceiver *, int, char *, int);
  Event(EventReceiver *, int);
  Event(EventReceiver *, int, int);
  Event(EventReceiver *, int, std::string);
  EventReceiver * getReceiver() const;
  int getType() const;
  char * getData() const;
  int getDataLen() const;
  int getInterval() const;
  std::string getStrData() const;
};
