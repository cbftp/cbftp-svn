#include "iomanager.h"

IOManager::IOManager() {
  epollfd = epoll_create(100);
  wm = global->getWorkManager();
  blockpool = wm->getBlockPool();
  blocksize = blockpool->blockSize();
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
  pthread_setname_np(thread, "Input");
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
  char buf[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, res->ai_addr, buf, INET_ADDRSTRLEN);
  addrmap[sockfd] = std::string(buf, INET_ADDRSTRLEN);
  typemap[sockfd] = 1;
  receivermap[sockfd] = er;
  struct epoll_event event;
  event.events = EPOLLOUT;
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
  typemap[sockfd] = 5;
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
  SSL_set_mode(ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
  sslmap[id] = ssl;
  SSL_set_fd(ssl, id);
  int ret = SSL_connect(ssl);
  if (ret > 0) { // probably wont happen :)
    typemap[id] = 4;
  }
  else {
    switch(SSL_get_error(ssl, ret)) {
      case SSL_ERROR_WANT_READ:
        break;
      case SSL_ERROR_WANT_WRITE:
        break;
    }
  }
}

void IOManager::sendData(int id, std::string data) {
  char * buf = (char *) data.c_str();
  sendData(id, buf, data.length());
}

void IOManager::sendData(int id, char * buf, unsigned int buflen) {
  int b_sent;
  SSL * ssl;
  char * datablock;
  switch(typemap[id]) {
    case 1: // tcp plain
      b_sent = send(id, buf, buflen, 0);
      break;
    case 2: // tcp ssl negotiation
    case 3: // tcp ssl negotiation
      datablock = blockpool->getBlock();
      memcpy(datablock, buf, buflen);
      sendqueuemap[id].push_back(DataBlock(datablock, buflen));
      break;
    case 4: // tcp ssl
      ssl = sslmap[id];
      b_sent = SSL_write(ssl, buf, buflen);
      if (b_sent < 0) {
        int code = SSL_get_error(ssl, b_sent);
        if (code == SSL_ERROR_WANT_READ || code == SSL_ERROR_WANT_WRITE) {
          typemap[id] = 3;
          datablock = blockpool->getBlock();
          memcpy(datablock, buf, buflen);
          sendqueuemap[id].push_back(DataBlock(datablock, buflen));
        }
      }
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
    case 3:
    case 4:
      SSL_shutdown(sslmap[id]);
      close(id);
      break;
    case 5: // udp
      close(id);
      break;
  }
  typemap[id] = -1;
}

std::string IOManager::getCipher(int id) {
  const char * cipher = SSL_CIPHER_get_name(SSL_get_current_cipher(sslmap[id]));
  return std::string(cipher);
}

std::string IOManager::getSocketAddress(int id) {
  if (addrmap.find(id) != addrmap.end()) {
    return addrmap[id];
  }
  return "";
}

void IOManager::runInstance() {
  struct epoll_event * events;
  events = (epoll_event *) calloc(MAXEVENTS, sizeof (struct epoll_event));
  int fds;
  int currfd;
  int b_recv;
  int i;
  char * buf;
  SSL * ssl;
  EventReceiver * er;
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
            if (b_recv <= 0) {
              blockpool->returnBlock(buf);
              wm->dispatchEventDisconnected(er);
              closeSocket(currfd);
              break;
            }
            wm->dispatchFDData(er, buf, b_recv);
          }
          else if (events[i].events & EPOLLOUT) { // socket connected
            wm->dispatchEventConnected(er);
            struct epoll_event event;
            event.events = EPOLLIN;
            event.data.fd = currfd;
            epoll_ctl(epollfd, EPOLL_CTL_MOD, currfd, &event);
          }
          break;
        case 2: // tcp ssl redo connect
          if (events[i].events & EPOLLIN) { // incoming data
            ssl = sslmap[currfd];
            b_recv = SSL_connect(ssl);
            if (b_recv > 0) {
              typemap[currfd] = 4;
              wm->dispatchEventSSLSuccess(er);
              while (sendqueuemap[currfd].size() > 0) {
                DataBlock sendblock = sendqueuemap[currfd].front();
                b_recv = SSL_write(ssl, sendblock.data(), sendblock.dataLength());
                if (b_recv > 0) {
                  blockpool->returnBlock(sendblock.data());
                  sendqueuemap[currfd].pop_front();
                }
                else {
                  typemap[currfd] = 3;
                  break;
                }
              }
            }
            else if (b_recv == 0) {
              wm->dispatchEventDisconnected(er);
              closeSocket(currfd);
              break;
            }
            else {
              switch(SSL_get_error(ssl, b_recv)) {
                case SSL_ERROR_WANT_READ:
                  break;
                case SSL_ERROR_WANT_WRITE:
                  break;
              }
            }
          }
          break;
        case 3: // tcp ssl redo write
          if (events[i].events & EPOLLIN) { // incoming data
            ssl = sslmap[currfd];
            DataBlock sendblock = sendqueuemap[currfd].front();
            b_recv = SSL_write(ssl, sendblock.data(), sendblock.dataLength());
            if (b_recv > 0) {
              blockpool->returnBlock(sendblock.data());
              sendqueuemap[currfd].pop_front();
              typemap[currfd] = 4;
              while (sendqueuemap[currfd].size() > 0) {
                sendblock = sendqueuemap[currfd].front();
                b_recv = SSL_write(ssl, sendblock.data(), sendblock.dataLength());
                if (b_recv > 0) {
                  blockpool->returnBlock(sendblock.data());
                  sendqueuemap[currfd].pop_front();
                }
                else {
                  typemap[currfd] = 3;
                  break;
                }
              }
            }
            else if (b_recv == 0) {
              wm->dispatchEventDisconnected(er);
              closeSocket(currfd);
              break;
            }
          }
          break;
        case 4: // tcp ssl
          if (events[i].events & EPOLLIN) { // incoming data
            ssl = sslmap[currfd];
            while (true) {
              buf = blockpool->getBlock();
              b_recv = SSL_read(ssl, buf, blocksize);
              if (b_recv < 0) {
                switch(SSL_get_error(ssl, b_recv)) {
                  case SSL_ERROR_WANT_WRITE:
                    break;
                  case SSL_ERROR_WANT_READ:
                    break;
                }
                blockpool->returnBlock(buf);
                break;
              }
              else if (b_recv == 0) {
                blockpool->returnBlock(buf);
                wm->dispatchEventDisconnected(er);
                closeSocket(currfd);
                break;
              }
              wm->dispatchFDData(er, buf, b_recv);
            }
          }
          break;
        case 5: // udp
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
