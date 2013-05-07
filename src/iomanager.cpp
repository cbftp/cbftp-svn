#include "iomanager.h"

IOManager::IOManager() {
  epollfd = epoll_create(100);
  wm = global->getWorkManager();
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
  pthread_setname_np(thread, "InputOutput");
}

int IOManager::registerTCPClientSocket(EventReceiver * er, std::string addr, int port) {
  struct addrinfo sock, *res;
  memset(&sock, 0, sizeof(sock));
  sock.ai_family = AF_UNSPEC;
  sock.ai_socktype = SOCK_STREAM;
  getaddrinfo(addr.data(), global->int2Str(port).data(), &sock, &res);
  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  connect(sockfd, res->ai_addr, res->ai_addrlen);
  typemap[sockfd] = 1;
  receivermap[sockfd] = er;
  struct epoll_event event;
  event.events = EPOLLIN | EPOLLOUT;
  event.data.fd = sockfd;
  epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
  return sockfd;
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

  struct addrinfo sock, *res;
  memset(&sock, 0, sizeof(sock));
  sock.ai_family = AF_UNSPEC;
  sock.ai_socktype = SOCK_DGRAM;
  sock.ai_protocol = IPPROTO_UDP;
  getaddrinfo("0.0.0.0", global->int2Str(port).data(), &sock, &res);
  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  bind(sockfd, res->ai_addr, res->ai_addrlen);
  typemap[sockfd] = 3;
  receivermap[sockfd] = er;
  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = sockfd;
  epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
  return sockfd;
}

void IOManager::negotiateSSL(int id) {
  if (typemap[id] != 1) {
    return;
  }
  typemap[id] = 2;
  SSL * ssl = SSL_new(global->getSSLCTX());
  SSL_set_fd(ssl, id);
  SSL_connect(ssl);
  sslmap[id] = ssl;
}

void IOManager::send(int id, std::string data) {
  switch(typemap[id]) {
    case 1: // tcp plain
      break;
    case 2: // tcp ssl

      break;
  }
}

void IOManager::closeSocket(int id) {
  switch(typemap[id]) {
    case 0: // keyboard
      break; // one does not simply close the keyboard input
    case 1: // tcp plain
      close(id);
      break;
    case 2: // tcp ssl
      SSL_shutdown(sslmap[id]);
      close(id);
      break;
    case 3: // udp
      close(id);
      break;
  }
}

void IOManager::runInstance() {
  struct epoll_event * events;
  events = (epoll_event *) calloc(MAXEVENTS, sizeof (struct epoll_event));
  int fds;
  int currfd;
  int b_recv;
  int i;
  char * buf;
  EventReceiver * er;
  DataBlockPool * blockpool = wm->getBlockPool();
  int blocksize = blockpool->blockSize();
  while(1) {
    fds = epoll_wait(epollfd, events, MAXEVENTS, -1);
    for (i = 0; i < fds; i++) {
      currfd = events[i].data.fd;
      er = receivermap[currfd];
      switch (typemap[currfd]) {
        case 0: // keyboard
          wm->dispatchFDData(er);
          break;
        case 1: // tcp plain
          if (events[i].events & EPOLLIN) { // incoming data
            buf = blockpool->getBlock();
            b_recv = recv(currfd, buf, blocksize, 0);
            wm->dispatchFDData(er, buf, b_recv);
          }
          break;
        case 2: // tcp ssl
          if (events[i].events & EPOLLIN) { // incoming data
            buf = blockpool->getBlock();
            b_recv = SSL_read(sslmap[currfd], buf, blocksize);
            wm->dispatchFDData(er, buf, b_recv);
          }
          break;
        case 3: // udp
          if (events[i].events & EPOLLIN) { // incoming data
            buf = blockpool->getBlock();
            b_recv = recvfrom(currfd, buf, blocksize, 0, (struct sockaddr *) 0, (socklen_t *) 0);
            wm->dispatchFDData(er, buf, b_recv);
          }
          break;
      }
    }
    if (!fds) { // timeout occurred

    }
  }
}

void * IOManager::run(void * arg) {
  ((IOManager *) arg)->runInstance();
  return NULL;
}
