#pragma once

class EventReceiver;

class Event {
private:
  EventReceiver * receiver;
  int type;
  char * data;
  int datalen;
  int interval;
public:
  Event(EventReceiver *, int, char *, int);
  Event(EventReceiver *, int);
  Event(EventReceiver *, int, int);
  EventReceiver * getReceiver() const;
  int getType() const;
  char * getData() const;
  int getDataLen() const;
  int getInterval() const;
};
