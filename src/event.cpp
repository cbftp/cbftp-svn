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

Event::Event(EventReceiver * er, int type, int numdata) {
  this->receiver = er;
  this->type = type;
  this->numdata = numdata;
}

Event::Event(EventReceiver * er, int type, std::string error) {
  this->receiver = er;
  this->type = type;
  this->strdata = error;
}

EventReceiver * Event::getReceiver() const {
  return receiver;
}

int Event::getType() const {
  return type;
}

char * Event::getData() const {
  return data;
}

int Event::getDataLen() const {
  return datalen;
}

int Event::getNumericalData() const {
  return numdata;
}

std::string Event::getStrData() const {
  return strdata;
}
