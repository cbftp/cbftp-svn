#include "asynctask.h"

AsyncTask::AsyncTask(EventReceiver * er, int type, void (*taskfunction)(int), int data) :
  receiver(er), type(type), taskfunction(taskfunction), dataispointer(false), data(data)
{
}

AsyncTask::AsyncTask(EventReceiver * er, int type, void (*taskfunction)(void *), void * data) :
  receiver(er), type(type), taskfunctionp(taskfunction), dataispointer(true), datap(data)
{
}

void AsyncTask::execute() {
  if (dataispointer) {
    taskfunctionp(datap);
  }
  else {
    taskfunction(data);
  }
}

EventReceiver * AsyncTask::getReceiver() const {
  return receiver;
}

bool AsyncTask::dataIsPointer() const {
  return dataispointer;
}

int AsyncTask::getType() const {
  return type;
}

void * AsyncTask::getData() const {
  return datap;
}

int AsyncTask::getNumData() const {
  return data;
}
