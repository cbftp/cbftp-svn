#pragma once

class EventReceiver;

class AsyncTask {
private:
  EventReceiver * receiver;
  int type;
  void (*taskfunction)(EventReceiver *, int);
  void (*taskfunctionp)(EventReceiver *, void *);
  bool dataispointer;
  int data;
  void * datap;
public:
  AsyncTask(EventReceiver *, int, void (*)(EventReceiver *, int), int);
  AsyncTask(EventReceiver *, int, void (*)(EventReceiver *, void *), void *);
  void execute();
  EventReceiver * getReceiver() const;
  int getType() const;
  bool dataIsPointer() const;
  void * getData() const;
  int getNumData() const;
};
