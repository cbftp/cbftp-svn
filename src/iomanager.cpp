#include "iomanager.h"

IOManager::IOManager() {
  epollfd = epoll_create(100);
  wm = global->getWorkManager();
  blockpool = wm->getBlockPool();
  blocksize = blockpool->blockSize();
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
#ifdef _ISOC95_SOURCE
  pthread_setname_np(thread, "Input");
#endif
}

int IOManager::registerTCPClientSocket(EventReceiver * er, std::string addr, int port) {
  struct addrinfo sock, *res;
  memset(&sock, 0, sizeof(sock));
  sock.ai_family = AF_INET;
  sock.ai_socktype = SOCK_STREAM;
  int status = getaddrinfo(addr.data(), global->int2Str(port).data(), &sock, &res);
  if (status != 0) {
    er->FDFail("Failed to resolve DNS. Error code: " + global->int2Str(status));
    return -1;
  }
  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  connect(sockfd, res->ai_addr, res->ai_addrlen);
  char buf[res->ai_addrlen];
  struct sockaddr_in* saddr = (struct sockaddr_in*)res->ai_addr;
  inet_ntop(AF_INET, &(saddr->sin_addr), buf, res->ai_addrlen);
  addrmap[sockfd] = std::string(buf);
  typemap[sockfd] = FD_TCP_PLAIN;
  receivermap[sockfd] = er;
  struct epoll_event event;
  event.events = EPOLLOUT;
  event.data.fd = sockfd;
  epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
  return sockfd;
}

int IOManager::registerTCPServerSocket(EventReceiver * er, int port) {
  struct addrinfo sock, *res;
  memset(&sock, 0, sizeof(sock));
  sock.ai_family = AF_INET;
  sock.ai_socktype = SOCK_STREAM;
  getaddrinfo("0.0.0.0", global->int2Str(port).data(), &sock, &res);
  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  bind(sockfd, res->ai_addr, res->ai_addrlen);
  typemap[sockfd] = FD_TCP_SERVER;
  receivermap[sockfd] = er;
  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = sockfd;
  epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
  return sockfd;
}

void IOManager::registerStdin(EventReceiver * er) {
  typemap[STDIN_FILENO] = FD_KEYBOARD;
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
  typemap[sockfd] = FD_UDP;
  receivermap[sockfd] = er;
  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = sockfd;
  epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
  return sockfd;
}

void IOManager::negotiateSSL(int id) {
  if (typemap[id] != FD_TCP_PLAIN) {
    return;
  }
  typemap[id] = FD_TCP_SSL_NEG_REDO_CONN;
  SSL * ssl = SSL_new(global->getSSLCTX());
  SSL_set_mode(ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
  sslmap[id] = ssl;
  SSL_set_fd(ssl, id);
  int ret = SSL_connect(ssl);
  if (ret > 0) { // probably wont happen :)
    typemap[id] = FD_TCP_SSL;
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
    case FD_TCP_PLAIN: // tcp plain
      b_sent = send(id, buf, buflen, 0);
      break;
    case FD_TCP_SSL_NEG_REDO_CONN: // tcp ssl negotiation
    case FD_TCP_SSL_NEG_REDO_WRITE: // tcp ssl negotiation
      datablock = blockpool->getBlock();
      memcpy(datablock, buf, buflen);
      sendqueuemap[id].push_back(DataBlock(datablock, buflen));
      break;
    case FD_TCP_SSL: // tcp ssl
      ssl = sslmap[id];
      b_sent = SSL_write(ssl, buf, buflen);
      if (b_sent < 0) {
        int code = SSL_get_error(ssl, b_sent);
        if (code == SSL_ERROR_WANT_READ || code == SSL_ERROR_WANT_WRITE) {
          typemap[id] = FD_TCP_SSL_NEG_REDO_WRITE;
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
    case FD_KEYBOARD: // keyboard
      break; // one does not simply close the keyboard input
    case FD_TCP_PLAIN: // tcp plain
      close(id);
      break;
    case FD_TCP_SSL_NEG_REDO_CONN: // tcp ssl
    case FD_TCP_SSL_NEG_REDO_WRITE:
    case FD_TCP_SSL:
      SSL_shutdown(sslmap[id]);
      close(id);
      break;
    case FD_UDP: // udp
      close(id);
      break;
    case FD_TCP_SERVER: // tcp server
      close(id);
      break;
  }
  typemap[id] = FD_UNUSED;
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
        case FD_KEYBOARD: // keyboard
          wm->dispatchFDData(er);
          break;
        case FD_TCP_PLAIN: // tcp plain
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
        case FD_TCP_SSL_NEG_REDO_CONN: // tcp ssl redo connect
          if (events[i].events & EPOLLIN) { // incoming data
            ssl = sslmap[currfd];
            b_recv = SSL_connect(ssl);
            if (b_recv > 0) {
              typemap[currfd] = FD_TCP_SSL;
              wm->dispatchEventSSLSuccess(er);
              while (sendqueuemap[currfd].size() > 0) {
                DataBlock sendblock = sendqueuemap[currfd].front();
                b_recv = SSL_write(ssl, sendblock.data(), sendblock.dataLength());
                if (b_recv > 0) {
                  blockpool->returnBlock(sendblock.data());
                  sendqueuemap[currfd].pop_front();
                }
                else {
                  typemap[currfd] = FD_TCP_SSL_NEG_REDO_WRITE;
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
        case FD_TCP_SSL_NEG_REDO_WRITE: // tcp ssl redo write
          if (events[i].events & EPOLLIN) { // incoming data
            ssl = sslmap[currfd];
            DataBlock sendblock = sendqueuemap[currfd].front();
            b_recv = SSL_write(ssl, sendblock.data(), sendblock.dataLength());
            if (b_recv > 0) {
              blockpool->returnBlock(sendblock.data());
              sendqueuemap[currfd].pop_front();
              typemap[currfd] = FD_TCP_SSL;
              while (sendqueuemap[currfd].size() > 0) {
                sendblock = sendqueuemap[currfd].front();
                b_recv = SSL_write(ssl, sendblock.data(), sendblock.dataLength());
                if (b_recv > 0) {
                  blockpool->returnBlock(sendblock.data());
                  sendqueuemap[currfd].pop_front();
                }
                else {
                  typemap[currfd] = FD_TCP_SSL_NEG_REDO_WRITE;
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
        case FD_TCP_SSL: // tcp ssl
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
        case FD_UDP: // udp
          if (events[i].events & EPOLLIN) { // incoming data
            buf = blockpool->getBlock();
            b_recv = recvfrom(currfd, buf, blocksize, 0, (struct sockaddr *) 0, (socklen_t *) 0);
            wm->dispatchFDData(er, buf, b_recv);
          }
          break;
        case FD_TCP_SERVER: // tcp server
          if (events[i].events & EPOLLIN) { // incoming connection

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
