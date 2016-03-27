#include "eventreceiver.h"

EventReceiver::~EventReceiver() {

}

void EventReceiver::tick(int message) {

}

void EventReceiver::signal(int sig, int value) {

}

void EventReceiver::FDNew(int sockid) {

}

void EventReceiver::FDConnecting(int sockid, std::string addr) {

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

void EventReceiver::asyncTaskComplete(int type, void * data) {

}

void EventReceiver::asyncTaskComplete(int type, int) {

}
