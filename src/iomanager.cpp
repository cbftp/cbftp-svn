#include "iomanager.h"

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <vector>
#include <openssl/err.h>
#include <errno.h>

#include "workmanager.h"
#include "globalcontext.h"
#include "datablock.h"
#include "datablockpool.h"
#include "eventlog.h"
#include "datafilehandler.h"
#include "tickpoke.h"

IOManager::IOManager() {
  epollfd = epoll_create(100);
  wm = global->getWorkManager();
  blockpool = wm->getBlockPool();
  blocksize = blockpool->blockSize();
  hasdefaultinterface = false;
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
#ifdef _ISOC95_SOURCE
  pthread_setname_np(thread, "Input");
#endif
  global->getTickPoke()->startPoke(this, "EventReceiver", TICKPERIOD, 0);
}

void IOManager::tick(int message) {
  bool closefd = false;
  std::map<int, int>::iterator it;
  for (it = connecttimemap.begin(); it != connecttimemap.end(); it++) {
    it->second += TICKPERIOD;
    if (it->second >= TIMEOUT_MS) {
      it->second = -1;
      closefd = true;
    }
  }
  if (closefd) {
    bool changes = true;
    while (changes) {
      changes = false;
      for (it = connecttimemap.begin(); it != connecttimemap.end(); it++) {
        if (it->second == -1) {
          int sockfd = it->first;
          closeSocket(sockfd);
          changes = true;
          wm->dispatchEventFail(receivermap[sockfd], "Connection timeout");
          break;
        }
      }
    }
  }
}

int IOManager::registerTCPClientSocket(EventReceiver * er, std::string addr, int port, int * sockfdp) {
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
  *sockfdp = sockfd;
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  if (hasDefaultInterface()) {
    struct addrinfo sock2, *res2;
    memset(&sock2, 0, sizeof(sock2));
    sock2.ai_family = AF_INET;
    sock2.ai_socktype = SOCK_STREAM;
    getaddrinfo(getInterfaceAddress(getDefaultInterface()).data(), "0", &sock2, &res2);
    bind(sockfd, res2->ai_addr, res2->ai_addrlen);
  }
  connect(sockfd, res->ai_addr, res->ai_addrlen);
  char buf[res->ai_addrlen];
  struct sockaddr_in* saddr = (struct sockaddr_in*)res->ai_addr;
  inet_ntop(AF_INET, &(saddr->sin_addr), buf, res->ai_addrlen);
  addrmap[sockfd] = std::string(buf);
  typemap[sockfd] = FD_TCP_CONNECTING;
  receivermap[sockfd] = er;
  connecttimemap[sockfd] = 0;
  struct epoll_event event;
  event.events = EPOLLOUT;
  event.data.fd = sockfd;
  epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
  return sockfd;
}

int IOManager::registerTCPServerSocket(EventReceiver * er, int port) {
  return registerTCPServerSocket(er, port, false);
}

int IOManager::registerTCPServerSocket(EventReceiver * er, int port, bool local) {
  struct addrinfo sock, *res;
  memset(&sock, 0, sizeof(sock));
  sock.ai_family = AF_INET;
  sock.ai_socktype = SOCK_STREAM;
  std::string addr = "0.0.0.0";
  if (local) {
    addr = "127.0.0.1";
  }
  getaddrinfo(addr.c_str(), global->int2Str(port).data(), &sock, &res);
  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  int yes = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  bind(sockfd, res->ai_addr, res->ai_addrlen);
  listen(sockfd, 10);
  typemap[sockfd] = FD_TCP_SERVER;
  receivermap[sockfd] = er;
  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = sockfd;
  epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
  return sockfd;
}

void IOManager::registerTCPServerClientSocket(EventReceiver * er, int sockfd) {
  typemap[sockfd] = FD_TCP_PLAIN_LISTEN;
  receivermap[sockfd] = er;
  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = sockfd;
  epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event);
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
  std::string addr = "0.0.0.0";
  getaddrinfo(addr.c_str(), global->int2Str(port).data(), &sock, &res);
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

void IOManager::negotiateSSLConnect(int id) {
  negotiateSSLConnect(id, NULL);
}

void IOManager::negotiateSSLConnect(int id, EventReceiver * er) {
  if (typemap[id] == FD_TCP_PLAIN || typemap[id] == FD_TCP_PLAIN_LISTEN) {
    typemap[id] = FD_TCP_SSL_NEG_REDO_CONN;
  }
  else {
    return;
  }
  negotiateSSL(id, er);
}

void IOManager::negotiateSSLAccept(int id) {
  if (typemap[id] == FD_TCP_PLAIN || typemap[id] == FD_TCP_PLAIN_LISTEN) {
    typemap[id] = FD_TCP_SSL_NEG_REDO_ACCEPT;
  }
  else {
    return;
  }
  negotiateSSL(id, NULL);
}

void IOManager::negotiateSSL(int id, EventReceiver * er) {
  struct epoll_event event;
  event.events = EPOLLIN;
  event.data.fd = id;
  epoll_ctl(epollfd, EPOLL_CTL_DEL, id, &event);
  SSL * ssl = SSL_new(global->getSSLCTX());
  SSL_set_mode(ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
  sslmap[id] = ssl;
  SSL_set_fd(ssl, id);
  if (er != NULL) {
    int parentid = -1;
    for (std::map<int, EventReceiver *>::iterator it = receivermap.begin(); it != receivermap.end(); it++) {
      if (it->second == er) {
        parentid = it->first;
        break;
      }
    }
    if (parentid != -1) {
      SSL_copy_session_id(ssl, sslmap[parentid]);
    }
  }
  int ret = -1;
  if (typemap[id] == FD_TCP_SSL_NEG_REDO_CONN) {
    ret = SSL_connect(ssl);
  }
  else {
    ret = SSL_accept(ssl);
  }
  if (ret > 0) { // probably wont happen :)
    typemap[id] = FD_TCP_SSL;
    wm->dispatchEventSSLSuccess(receivermap[id]);
  }
  else {
    if (!investigateSSLError(SSL_get_error(ssl, ret), id, ret)) {
      wm->dispatchEventDisconnected(receivermap[id]);
      closeSocket(id);
    }
  }
  event.events = EPOLLIN;
  epoll_ctl(epollfd, EPOLL_CTL_ADD, id, &event);
}

void IOManager::forceSSLhandshake(int id) {
  SSL * ssl = sslmap[id];
  int ret = SSL_do_handshake(ssl);
  typemap[id] = FD_TCP_SSL_NEG_REDO_HANDSHAKE;
  if (ret > 0) { // probably wont happen :)
    typemap[id] = FD_TCP_SSL;
    wm->dispatchEventSSLSuccess(receivermap[id]);
  }
  else {
    if (!investigateSSLError(SSL_get_error(ssl, ret), id, ret)) {
      wm->dispatchEventDisconnected(receivermap[id]);
      closeSocket(id);
    }
  }
}

bool IOManager::investigateSSLError(int error, int currfd, int b_recv) {
  switch(error) {
    case SSL_ERROR_WANT_READ:
      return true;
    case SSL_ERROR_WANT_WRITE:
      return true;
    case SSL_ERROR_SYSCALL:
      if (errno == EAGAIN) {
        struct epoll_event event;
        event.events = EPOLLOUT;
        event.data.fd = currfd;
        epoll_ctl(epollfd, EPOLL_CTL_MOD, currfd, &event);
        return true;
      }
      break;
  }
  unsigned long e = ERR_get_error();
  global->getEventLog()->log("IOManager", "SSL error on connection to " +
      addrmap[currfd] + ": " +
      global->int2Str(error) + " return code: " + global->int2Str(b_recv) +
      " errno: " + strerror(errno) +
      (e ? " String: " + std::string(ERR_error_string(e, NULL)) : ""));
  return false;
}

void IOManager::sendData(int id, std::string data) {
  char * buf = (char *) data.c_str();
  sendData(id, buf, data.length());
}

void IOManager::sendData(int id, const char * buf, unsigned int buflen) {
  int b_sent;
  SSL * ssl;
  char * datablock;
  switch(typemap[id]) {
    case FD_TCP_PLAIN: // tcp plain
    case FD_TCP_PLAIN_LISTEN:
      b_sent = send(id, buf, buflen, 0);
      break;
    case FD_TCP_CONNECTING:
    case FD_TCP_SSL_NEG_REDO_CONN: // tcp ssl negotiation
    case FD_TCP_SSL_NEG_REDO_WRITE: // tcp ssl negotiation
    case FD_TCP_SSL_NEG_REDO_ACCEPT: // tcp ssl negotiation
    case FD_TCP_SSL_NEG_REDO_HANDSHAKE: // tcp ssl negotiation
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
    case FD_TCP_CONNECTING:
      connecttimemap.erase(id);
      close(id);
      break;
    case FD_TCP_PLAIN: // tcp plain
    case FD_TCP_PLAIN_LISTEN:
      close(id);
      break;
    case FD_TCP_SSL_NEG_REDO_CONN: // tcp ssl
    case FD_TCP_SSL_NEG_REDO_WRITE:
    case FD_TCP_SSL_NEG_REDO_ACCEPT:
    case FD_TCP_SSL_NEG_REDO_HANDSHAKE:
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

std::string IOManager::getSocketAddress(int id) const {
  std::map<int, std::string>::const_iterator it = addrmap.find(id);
  if (it != addrmap.end()) {
    return it->second;
  }
  return "";
}

void IOManager::runInstance() {
  struct epoll_event * events;
  events = (epoll_event *) calloc(MAXEVENTS, sizeof (struct epoll_event));
  int fds;
  int currfd;
  int newfd;
  int b_recv;
  int i;
  char * buf;
  SSL * ssl;
  struct sockaddr addr;
  socklen_t addrlen;
  EventReceiver * er;
  while(1) {
    fds = epoll_wait(epollfd, events, MAXEVENTS, -1);
    for (i = 0; i < fds; i++) {
      currfd = events[i].data.fd;
      er = receivermap[currfd];
      if (events[i].events & EPOLLOUT) {
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.fd = currfd;
        epoll_ctl(epollfd, EPOLL_CTL_MOD, currfd, &event);
        if (typemap[currfd] == FD_TCP_CONNECTING) {
          unsigned int error;
          unsigned int errorlen = sizeof(error);
          getsockopt(currfd, SOL_SOCKET, SO_ERROR, &error, &errorlen);
          if (error == ECONNREFUSED) {
            closeSocket(currfd);
            wm->dispatchEventFail(er, "Connection refused");
            continue;
          }
          else if (error == EHOSTUNREACH) {
            closeSocket(currfd);
            wm->dispatchEventFail(er, "No route to host");
            continue;
          }
          typemap[currfd] = FD_TCP_PLAIN;
          connecttimemap.erase(currfd);
          wm->dispatchEventConnected(er);
          continue;
        }
        events[i].events |= EPOLLIN;
      }
      switch (typemap[currfd]) {
        case FD_KEYBOARD: // keyboard
          wm->dispatchFDData(er);
          break;
        case FD_TCP_PLAIN: // tcp plain
        case FD_TCP_PLAIN_LISTEN:
          if (events[i].events & EPOLLIN) { // incoming data
            buf = blockpool->getBlock();
            b_recv = recv(currfd, buf, blocksize, 0);
            if (b_recv <= 0) {
              blockpool->returnBlock(buf);
              closeSocket(currfd);
              wm->dispatchEventDisconnected(er);
              break;
            }
            wm->dispatchFDData(er, buf, b_recv);
          }
          break;
        case FD_TCP_SSL_NEG_REDO_CONN: // tcp ssl redo connect
        case FD_TCP_SSL_NEG_REDO_ACCEPT: // tcp ssl redo accept
        case FD_TCP_SSL_NEG_REDO_HANDSHAKE: // tcp ssl redo handshake
          if (events[i].events & EPOLLIN) { // incoming data
            ssl = sslmap[currfd];
            if (typemap[currfd] == FD_TCP_SSL_NEG_REDO_CONN) {
              b_recv = SSL_connect(ssl);
            }
            else if (typemap[currfd] == FD_TCP_SSL_NEG_REDO_ACCEPT) {
              b_recv = SSL_accept(ssl);
            }
            else {
              b_recv = SSL_do_handshake(ssl);
            }
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
              if (!investigateSSLError(SSL_get_error(ssl, b_recv), currfd, b_recv)) {
                wm->dispatchEventDisconnected(er);
                closeSocket(currfd);
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
            }
            else if (b_recv < 0) {
              if (!investigateSSLError(SSL_get_error(ssl, b_recv), currfd, b_recv)) {
                wm->dispatchEventDisconnected(er);
                closeSocket(currfd);
              }
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
                if (!investigateSSLError(SSL_get_error(ssl, b_recv), currfd, b_recv)) {
                  wm->dispatchEventDisconnected(er);
                  closeSocket(currfd);
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
            newfd = accept(currfd, &addr, &addrlen);
            fcntl(newfd, F_SETFL, O_NONBLOCK);
            wm->dispatchEventNew(er, newfd);
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

std::list<std::pair<std::string, std::string> > IOManager::listInterfaces() {
  std::list<std::pair<std::string, std::string> > addrs;
  struct ifaddrs *ifaddr, *ifa;
  int family, s;
  char host[NI_MAXHOST];
  if (getifaddrs(&ifaddr) == -1) {
    global->getEventLog()->log("IOManager", "ERROR: Failed to list network interfaces");
    return addrs;
  }
  for (ifa = ifaddr; ifa != NULL && ifa->ifa_addr != NULL; ifa = ifa->ifa_next) {
    family = ifa->ifa_addr->sa_family;
    if (family == AF_INET) {
      s = getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),
          host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
      if (s != 0) {
        global->getEventLog()->log("IOManager", "ERROR: getnameinfo() failed");
        continue;
      }
      addrs.push_back(std::pair<std::string, std::string>(ifa->ifa_name, host));
    }
  }
  freeifaddrs(ifaddr);
  return addrs;
}

std::string IOManager::getDefaultInterface() const {
  return defaultinterface;
}

void IOManager::setDefaultInterface(std::string interface) {
  if (getInterfaceAddress(interface) == "") {
    if (hasdefaultinterface) {
      hasdefaultinterface = false;
      global->getEventLog()->log("IOManager", "Default network interface removed");
    }
  }
  else {
    defaultinterface = interface;
    hasdefaultinterface = true;
    global->getEventLog()->log("IOManager", "Default network interface set to: " + interface);
  }
}

bool IOManager::hasDefaultInterface() const {
  return hasdefaultinterface;
}

std::string IOManager::getInterfaceAddress(std::string interface) {
  int fd;
  struct ifreq ifr;
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ - 1);
  if (ioctl(fd, SIOCGIFADDR, &ifr) < 0) {
    return "";
  }
  close(fd);
  return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}

void IOManager::readConfiguration() {
  std::vector<std::string> lines;
  global->getDataFileHandler()->getDataFor("IOManager", &lines);
  std::vector<std::string>::iterator it;
  std::string line;
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("defaultinterface")) {
      setDefaultInterface(value);
    }
  }
}

void IOManager::writeState() {
  global->getEventLog()->log("IOManager", "Writing state...");
  if (hasDefaultInterface()) {
    global->getDataFileHandler()->addOutputLine("IOManager", "defaultinterface=" + getDefaultInterface());
  }
}
