#include "eventreceiver.h"

#include <iostream>

EventReceiver::~EventReceiver() {

}

void EventReceiver::tick(int message) {

}

void EventReceiver::signal(int) {

}

void EventReceiver::FDNew(int fd) {

}

void EventReceiver::FDConnected() {

}

void EventReceiver::FDData() {

}

void EventReceiver::FDData(char * data, unsigned int len) {

}

void EventReceiver::FDDisconnected() {

}

void EventReceiver::FDFail(std::string error) {

}

void EventReceiver::FDSSLSuccess() {

}

void EventReceiver::FDSSLFail() {

}

void EventReceiver::FDSendComplete() {

}
