#include "event.h"

Event::Event(EventReceiver * er, int type, char * data, int datalen) {
  this->receiver = er;
  this->type = type;
  this->data = data;
  this->datalen = datalen;
}

Event::Event(EventReceiver * er, int type) {
  this->receiver = er;
  this->type = type;
}

Event::Event(EventReceiver * er, int type, int interval) {
  this->receiver = er;
  this->type = type;
  this->interval = interval;
}

EventReceiver * Event::getReceiver() {
  return receiver;
}

int Event::getType() {
  return type;
}

char * Event::getData() {
  return data;
}

int Event::getDataLen() {
  return datalen;
}

int Event::getInterval() {
  return interval;
}
