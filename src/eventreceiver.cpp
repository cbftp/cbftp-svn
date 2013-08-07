#include "eventreceiver.h"

EventReceiver::EventReceiver() {
  pthread_mutex_init(&eventlock, NULL);
}

EventReceiver::~EventReceiver() {

}

void EventReceiver::tick(int message) {

}

void EventReceiver::FDConnected() {

}

void EventReceiver::FDData() {

}

void EventReceiver::FDData(char * data, unsigned int len) {

}

void EventReceiver::FDDisconnected() {

}

void EventReceiver::FDSSLSuccess() {

}

void EventReceiver::FDSSLFail() {

}

void EventReceiver::lock() {
  pthread_mutex_lock(&eventlock);
}

void EventReceiver::unlock() {
  pthread_mutex_unlock(&eventlock);
}
