#include "iomanager.h"

IOManager::IOManager() {
  epollfd = epoll_create(100);
  wm = global->getWorkManager();
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
  pthread_setname_np(thread, "InputOutput");
}

int IOManager::registerTCPClientSocket(EventReceiver * er, std::string addr, int port) {
  return 0;
}

void IOManager::registerStdin(EventReceiver * er) {
  typemap[STDIN_FILENO] = 0;
  receivermap[STDIN_FILENO] = er;
  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = STDIN_FILENO;
  epoll_ctl(epollfd, EPOLL_CTL_ADD, STDIN_FILENO, &event);
}

int IOManager::registerUDPServerSocket(EventReceiver * er, int port) {
  return 0;
}

void IOManager::negotiateSSL(int id) {
  if (typemap[id] != 1) {
    return;
  }
  typemap[id] = 2;
}

void IOManager::send(int id, std::string data) {

}

void IOManager::closeSocket(int id) {

}

void IOManager::runInstance() {
  struct epoll_event * events;
  events = (epoll_event *) calloc(MAXEVENTS, sizeof (struct epoll_event));
  int fds;
  int currfd;
  int b_recv;
  int i;
  char buf[MAXDATASIZE];
  EventReceiver * er;
  while(1) {
    fds = epoll_wait(epollfd, events, MAXEVENTS, -1);
    if (fds > 0) {
      for (i = 0; i < fds; i++) {
        currfd = events[i].data.fd;
        er = receivermap[currfd];
        switch (typemap[currfd]) {
          case 0: // keyboard
            wm->dispatchFDData(er);
            break;
          case 1: // tcp plain
            if (events[i].events & EPOLLIN) { // incoming data
              b_recv = recv(currfd, buf, MAXDATASIZE, 0);
              wm->dispatchFDData(er, buf, b_recv);
            }
            break;
          case 2: // tcp ssl
            if (events[i].events & EPOLLIN) { // incoming data
              wm->dispatchFDData(er, buf, b_recv);
            }
            break;
          case 3: // udp
            if (events[i].events & EPOLLIN) { // incoming data
              wm->dispatchFDData(er, buf, b_recv);
            }
            break;
        }
      }
    }
    else { // timeout occurred

    }
  }
}

void * IOManager::run(void * arg) {
  ((IOManager *) arg)->runInstance();
  return NULL;
}
