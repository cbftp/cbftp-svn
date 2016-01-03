#include "eventreceiver.h"

#include <iostream>

EventReceiver::~EventReceiver() {

}

void EventReceiver::tick(int message) {

}

void EventReceiver::signal(int sig) {

}

void EventReceiver::FDNew(int sockid) {

}

void EventReceiver::FDConnected(int sockid) {

}

void EventReceiver::FDData(int sockid) {

}

void EventReceiver::FDData(int sockid, char * data, unsigned int len) {

}

void EventReceiver::FDDisconnected(int sockid) {

}

void EventReceiver::FDFail(int sockid, std::string error) {

}

void EventReceiver::FDSSLSuccess(int sockid) {

}

void EventReceiver::FDSSLFail(int sockid) {

}

void EventReceiver::FDSendComplete(int sockid) {

}
