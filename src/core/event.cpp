#include "event.h"

#include "eventreceiver.h"

Event::Event() {
}

Event::Event(EventReceiver * er, int type, int numdata, void * data, int datalen) :
  receiver(er),
  type(type),
  data(data),
  datalen(datalen),
  numdata(numdata)
{
}

Event::Event(EventReceiver * er, int type, int numdata, void * data) :
  receiver(er),
  type(type),
  data(data),
  numdata(numdata)
{
}

Event::Event(EventReceiver * er, int type) :
  receiver(er),
  type(type)
{
}

Event::Event(EventReceiver * er, int type, int numdata) :
  receiver(er),
  type(type),
  numdata(numdata)
{
}

Event::Event(EventReceiver * er, int type, int numdata, int numdata2) :
  receiver(er),
  type(type),
  numdata(numdata),
  numdata2(numdata2)
{
}

Event::Event(EventReceiver * er, int type, int numdata, std::string strdata) :
  receiver(er),
  type(type),
  numdata(numdata),
  strdata(strdata)
{
}

Event::Event(Pointer<EventReceiver> & er, int type) :
  preceiver(er),
  type(type)
{
}

Event::~Event() {

}

EventReceiver * Event::getReceiver() const {
  return receiver;
}

int Event::getType() const {
  return type;
}

void * Event::getData() const {
  return data;
}

int Event::getDataLen() const {
  return datalen;
}

int Event::getNumericalData() const {
  return numdata;
}

int Event::getNumericalData2() const {
  return numdata2;
}

std::string Event::getStrData() const {
  return strdata;
}
