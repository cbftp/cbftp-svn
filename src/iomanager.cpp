#include "iomanager.h"

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
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
#include "scopelock.h"
#include "util.h"

IOManager::IOManager() :
  wm(global->getWorkManager()),
  blockpool(wm->getBlockPool()),
  blocksize(blockpool->blockSize()),
  sockidcounter(0),
  hasdefaultinterface(false) {

}

void IOManager::init() {
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
#ifdef _ISOC95_SOURCE
  pthread_setname_np(thread, "Input");
#endif
  global->getTickPoke()->startPoke(this, "EventReceiver", TICKPERIOD, 0);
}

void IOManager::tick(int message) {
  bool closefd = false;
  ScopeLock lock(socketinfomaplock);
  std::map<int, int>::iterator it;
  for (it = connecttimemap.begin(); it != connecttimemap.end(); it++) {
    int & timeelapsed = it->second;
    timeelapsed += TICKPERIOD;
    if (timeelapsed >= TIMEOUT_MS) {
      timeelapsed = -1;
      closefd = true;
    }
  }
  if (closefd) {
    bool changes = true;
    while (changes) {
      changes = false;
      for (it = connecttimemap.begin(); it != connecttimemap.end(); it++) {
        if (it->second == -1) {
          changes = true;
          std::map<int, SocketInfo>::iterator it2 = socketinfomap.find(it->first);
          if (it2 != socketinfomap.end()) {
            EventReceiver * er = it2->second.receiver;
            wm->dispatchEventFail(er, "Connection timeout");
          }
          closeSocketIntern(it->first);
          break;
        }
      }
    }
  }
}

int IOManager::registerTCPClientSocket(EventReceiver * er, std::string addr, int port, int * sockidp) {
  struct addrinfo sock, *res;
  memset(&sock, 0, sizeof(sock));
  sock.ai_family = AF_INET;
  sock.ai_socktype = SOCK_STREAM;
  int retcode = getaddrinfo(addr.data(), util::int2Str(port).data(), &sock, &res);
  if (retcode < 0) {
    if (!handleError(er)) {
      return -1;
    }
  }
  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  if (hasDefaultInterface()) {
    struct addrinfo sock2, *res2;
    memset(&sock2, 0, sizeof(sock2));
    sock2.ai_family = AF_INET;
    sock2.ai_socktype = SOCK_STREAM;
    retcode = getaddrinfo(getInterfaceAddress(getDefaultInterface()).data(), "0", &sock2, &res2);
    if (retcode < 0) {
      if (!handleError(er)) {
        return -1;
      }
    }
    retcode = bind(sockfd, res2->ai_addr, res2->ai_addrlen);
    if (retcode < 0) {
      if (!handleError(er)) {
        return -1;
      }
    }
  }
  retcode = connect(sockfd, res->ai_addr, res->ai_addrlen);
  if (retcode < 0) {
    if (!handleError(er)) {
      return -1;
    }
  }
  char buf[res->ai_addrlen];
  struct sockaddr_in* saddr = (struct sockaddr_in*)res->ai_addr;
  inet_ntop(AF_INET, &(saddr->sin_addr), buf, res->ai_addrlen);
  ScopeLock lock(socketinfomaplock);
  int sockid = sockidcounter++;
  socketinfomap[sockid].fd = sockfd;
  socketinfomap[sockid].type = FD_TCP_CONNECTING;
  socketinfomap[sockid].addr = std::string(buf);
  socketinfomap[sockid].receiver = er;
  connecttimemap[sockid] = 0;
  sockfdidmap[sockfd] = sockid;
  *sockidp = sockid;
  polling.addFDOut(sockfd);
  return 0;
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
  int retcode = getaddrinfo(addr.c_str(), util::int2Str(port).data(), &sock, &res);
  if (retcode < 0) {
    if (!handleError(er)) {
      return -1;
    }
  }
  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  int yes = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  retcode = bind(sockfd, res->ai_addr, res->ai_addrlen);
  if (retcode < 0) {
    if (!handleError(er)) {
      return -1;
    }
  }
  retcode = listen(sockfd, 10);
  if (retcode < 0) {
    if (!handleError(er)) {
      return -1;
    }
  }
  ScopeLock lock(socketinfomaplock);
  int sockid = sockidcounter++;
  socketinfomap[sockid].fd = sockfd;
  socketinfomap[sockid].type = FD_TCP_SERVER;
  socketinfomap[sockid].receiver = er;
  sockfdidmap[sockfd] = sockid;
  polling.addFDIn(sockfd);
  return sockid;
}

void IOManager::registerTCPServerClientSocket(EventReceiver * er, int sockid) {
  ScopeLock lock(socketinfomaplock);
  socketinfomap[sockid].receiver = er;
  int sockfd = socketinfomap[sockid].fd;
  polling.addFDIn(sockfd);
}

void IOManager::registerStdin(EventReceiver * er) {
  ScopeLock lock(socketinfomaplock);
  int sockid = sockidcounter++;
  socketinfomap[sockid].fd = STDIN_FILENO;
  socketinfomap[sockid].type = FD_KEYBOARD;
  socketinfomap[sockid].receiver = er;
  sockfdidmap[STDIN_FILENO] = sockid;
  polling.addFDIn(STDIN_FILENO);
}

int IOManager::registerUDPServerSocket(EventReceiver * er, int port) {
  struct addrinfo sock, *res;
  memset(&sock, 0, sizeof(sock));
  sock.ai_family = AF_UNSPEC;
  sock.ai_socktype = SOCK_DGRAM;
  sock.ai_protocol = IPPROTO_UDP;
  std::string addr = "0.0.0.0";
  int retcode = getaddrinfo(addr.c_str(), util::int2Str(port).data(), &sock, &res);
  if (retcode < 0) {
    if (!handleError(er)) {
      return -1;
    }
  }
  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  int yes = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  retcode = bind(sockfd, res->ai_addr, res->ai_addrlen);
  if (retcode < 0) {
    if (!handleError(er)) {
      return -1;
    }
  }
  ScopeLock lock(socketinfomaplock);
  int sockid = sockidcounter++;
  socketinfomap[sockid].fd = sockfd;
  socketinfomap[sockid].type = FD_UDP;
  socketinfomap[sockid].receiver = er;
  sockfdidmap[sockfd] = sockid;
  polling.addFDIn(sockfd);
  return sockid;
}

void IOManager::negotiateSSLConnect(int id) {
  negotiateSSLConnect(id, NULL);
}

void IOManager::negotiateSSLConnect(int id, EventReceiver * er) {
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::iterator it = socketinfomap.find(id);
  if (it != socketinfomap.end() &&
     (it->second.type == FD_TCP_PLAIN || it->second.type == FD_TCP_PLAIN_LISTEN)) {
    it->second.type = FD_TCP_SSL_NEG_REDO_CONN;
    lock.unlock();
    negotiateSSL(id, er);
  }
}

void IOManager::negotiateSSLAccept(int id) {
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::iterator it = socketinfomap.find(id);
    if (it != socketinfomap.end() &&
       (it->second.type == FD_TCP_PLAIN || it->second.type == FD_TCP_PLAIN_LISTEN)) {
      it->second.type = FD_TCP_SSL_NEG_REDO_ACCEPT;
      lock.unlock();
      negotiateSSL(id, NULL);
  }
}

void IOManager::negotiateSSL(int id, EventReceiver * er) {
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::iterator it = socketinfomap.find(id);
  if (it == socketinfomap.end()) {
    return;
  }
  SocketInfo & socketinfo = it->second;
  polling.removeFDIn(socketinfo.fd);
  SSL * ssl = SSL_new(global->getSSLCTX());
  SSL_set_mode(ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
  socketinfo.ssl = ssl;
  SSL_set_fd(ssl, socketinfo.fd);
  if (er != NULL) {
    std::map<int, SocketInfo>::iterator it2;
    SocketInfo * parentsocketinfo = NULL;
    for (it2 = socketinfomap.begin(); it2 != socketinfomap.end(); it2++) {
      if (it2->second.receiver == er) {
        parentsocketinfo = &it2->second;
        break;
      }
    }
    if (parentsocketinfo != NULL) {
      SSL_copy_session_id(ssl, parentsocketinfo->ssl);
    }
  }
  int ret = -1;
  if (socketinfo.type == FD_TCP_SSL_NEG_REDO_CONN) {
    ret = SSL_connect(ssl);
  }
  else {
    ret = SSL_accept(ssl);
  }
  if (ret > 0) { // probably wont happen :)
    socketinfo.type = FD_TCP_SSL;
    wm->dispatchEventSSLSuccess(socketinfo.receiver);
  }
  else {
    if (!investigateSSLError(SSL_get_error(ssl, ret), id, ret)) {
      wm->dispatchEventDisconnected(socketinfo.receiver);
      closeSocketIntern(id);
    }
  }
  polling.addFDIn(socketinfo.fd);
}

void IOManager::forceSSLhandshake(int id) {
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::iterator it = socketinfomap.find(id);
  if (it == socketinfomap.end()) {
    return;
  }
  SocketInfo & socketinfo = it->second;
  SSL * ssl = socketinfo.ssl;
  int ret = SSL_do_handshake(ssl);
  socketinfo.type = FD_TCP_SSL_NEG_REDO_HANDSHAKE;
  if (ret > 0) { // probably wont happen :)
    socketinfo.type = FD_TCP_SSL;
    wm->dispatchEventSSLSuccess(socketinfo.receiver);
  }
  else {
    if (!investigateSSLError(SSL_get_error(ssl, ret), id, ret)) {
      wm->dispatchEventDisconnected(socketinfo.receiver);
      closeSocketIntern(id);
    }
  }
}

bool IOManager::handleError(EventReceiver * er) {
  if (errno == EINPROGRESS) {
    return true;
  }
  er->FDFail(strerror(errno));
  return false;
}

bool IOManager::investigateSSLError(int error, int sockid, int b_recv) {
  std::map<int, SocketInfo>::iterator it = socketinfomap.find(sockid);
  if (it == socketinfomap.end()) {
    return false;
  }
  SocketInfo & socketinfo = it->second;
  switch(error) {
    case SSL_ERROR_WANT_READ:
      return true;
    case SSL_ERROR_WANT_WRITE:
      return true;
    case SSL_ERROR_SYSCALL:
      if (errno == EAGAIN) {
        polling.setFDOut(socketinfo.fd);
        return true;
      }
      break;
  }
  unsigned long e = ERR_get_error();
  global->getEventLog()->log("IOManager", "SSL error on connection to " +
      it->second.addr + ": " +
      util::int2Str(error) + " return code: " + util::int2Str(b_recv) +
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
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::iterator it = socketinfomap.find(id);
  if (it == socketinfomap.end()) {
    return;
  }
  SocketInfo & socketinfo = it->second;
  switch(socketinfo.type) {
    case FD_TCP_PLAIN: // tcp plain
    case FD_TCP_PLAIN_LISTEN:
      b_sent = send(socketinfo.fd, buf, buflen, 0);
      break;
    case FD_TCP_CONNECTING:
    case FD_TCP_SSL_NEG_REDO_CONN: // tcp ssl negotiation
    case FD_TCP_SSL_NEG_REDO_WRITE: // tcp ssl negotiation
    case FD_TCP_SSL_NEG_REDO_ACCEPT: // tcp ssl negotiation
    case FD_TCP_SSL_NEG_REDO_HANDSHAKE: // tcp ssl negotiation
      datablock = blockpool->getBlock();
      memcpy(datablock, buf, buflen);
      socketinfo.sendqueue.push_back(DataBlock(datablock, buflen));
      break;
    case FD_TCP_SSL: // tcp ssl
      ssl = socketinfo.ssl;
      b_sent = SSL_write(ssl, buf, buflen);
      if (b_sent < 0) {
        int code = SSL_get_error(ssl, b_sent);
        if (code == SSL_ERROR_WANT_READ || code == SSL_ERROR_WANT_WRITE) {
          socketinfo.type = FD_TCP_SSL_NEG_REDO_WRITE;
          datablock = blockpool->getBlock();
          memcpy(datablock, buf, buflen);
          socketinfo.sendqueue.push_back(DataBlock(datablock, buflen));
        }
      }
      break;
  }
}

void IOManager::closeSocket(int id) {
  ScopeLock lock(socketinfomaplock);
  closeSocketIntern(id);
}

void IOManager::closeSocketIntern(int id) {
  std::map<int, SocketInfo>::iterator it = socketinfomap.find(id);
  if (it == socketinfomap.end()) {
    return;
  }
  SocketInfo & socketinfo = it->second;
  if (socketinfo.type == FD_KEYBOARD) {
    return;
  }
  close(socketinfo.fd);
  if (socketinfo.ssl) {
    SSL_free(socketinfo.ssl);
  }
  sockfdidmap.erase(socketinfo.fd);
  connecttimemap.erase(id);
  socketinfomap.erase(it);
}

const char * IOManager::getCipher(SSL * ssl) {
  return SSL_CIPHER_get_name(SSL_get_current_cipher(ssl));
}

std::string IOManager::getCipher(int id) const {
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::const_iterator it = socketinfomap.find(id);
  if (it == socketinfomap.end() || it->second.ssl == NULL || it->second.type != FD_TCP_SSL) {
    return "";
  }
  const char * cipher = getCipher(it->second.ssl);
  return std::string(cipher);
}

std::string IOManager::getSocketAddress(int id) const {
  std::string addr = "";
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::const_iterator it = socketinfomap.find(id);
  if (it != socketinfomap.end()) {
    addr = it->second.addr;
  }
  return addr;
}

void IOManager::runInstance() {
  int currfd;
  int newfd;
  int sockid;
  int b_recv;
  char * buf;
  SSL * ssl;
  struct sockaddr addr;
  socklen_t addrlen;
  EventReceiver * er;
  std::map<int, SocketInfo>::iterator it;
  std::map<int, int>::iterator idit;
  std::list<std::pair<int, PollEvent> > fds;
  std::list<std::pair<int, PollEvent> >::const_iterator pollit;
  PollEvent pollevent;
  ScopeLock lock(socketinfomaplock);
  while(1) {
    lock.unlock();
    blockpool->awaitFreeBlocks();
    polling.wait(fds);
    lock.lock();
    for (pollit = fds.begin(); pollit != fds.end(); pollit++) {
      currfd = pollit->first;
      pollevent = pollit->second;
      idit = sockfdidmap.find(currfd);
      if (idit == sockfdidmap.end()) {
        continue;
      }
      sockid = idit->second;
      it = socketinfomap.find(sockid);
      if (it == socketinfomap.end()) {
        continue;
      }
      SocketInfo & socketinfo = it->second;
      er = socketinfo.receiver;
      if (pollevent == POLLEVENT_OUT) {
        polling.setFDIn(currfd);
        if (socketinfo.type == FD_TCP_CONNECTING) {
          unsigned int error;
          unsigned int errorlen = sizeof(error);
          getsockopt(currfd, SOL_SOCKET, SO_ERROR, &error, &errorlen);
          if (error != 0) {
            closeSocketIntern(sockid);
            wm->dispatchEventFail(er, strerror(error));
            continue;
          }
          socketinfo.type = FD_TCP_PLAIN;
          connecttimemap.erase(sockid);
          wm->dispatchEventConnected(er);
        }
        continue;
      }
      switch (socketinfo.type) {
        case FD_KEYBOARD: // keyboard
          lock.unlock();
          wm->dispatchFDData(er);
          lock.lock();
          break;
        case FD_TCP_PLAIN: // tcp plain
        case FD_TCP_PLAIN_LISTEN:
          if (pollevent == POLLEVENT_IN) { // incoming data
            buf = blockpool->getBlock();
            b_recv = recv(currfd, buf, blocksize, 0);
            if (b_recv == 0) {
              blockpool->returnBlock(buf);
              closeSocketIntern(sockid);
              wm->dispatchEventDisconnected(er);
              break;
            }
            else if (b_recv < 0) {
              blockpool->returnBlock(buf);
              if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
              }
              closeSocketIntern(sockid);
              wm->dispatchEventDisconnected(er);
              global->getEventLog()->log("IOManager",
                  "Socket error on established connection to "
                  + it->second.addr + ": " + strerror(errno));
              break;
            }
            wm->dispatchFDData(er, buf, b_recv);
          }
          break;
        case FD_TCP_SSL_NEG_REDO_CONN: // tcp ssl redo connect
        case FD_TCP_SSL_NEG_REDO_ACCEPT: // tcp ssl redo accept
        case FD_TCP_SSL_NEG_REDO_HANDSHAKE: // tcp ssl redo handshake
          if (pollevent == POLLEVENT_IN) { // incoming data
            ssl = socketinfo.ssl;
            if (socketinfo.type == FD_TCP_SSL_NEG_REDO_CONN) {
              b_recv = SSL_connect(ssl);
            }
            else if (socketinfo.type == FD_TCP_SSL_NEG_REDO_ACCEPT) {
              b_recv = SSL_accept(ssl);
            }
            else {
              b_recv = SSL_do_handshake(ssl);
            }
            if (b_recv > 0) {
              socketinfo.type = FD_TCP_SSL;
              wm->dispatchEventSSLSuccess(er);
              while (socketinfo.sendqueue.size() > 0) {
                DataBlock sendblock = socketinfo.sendqueue.front();
                b_recv = SSL_write(ssl, sendblock.data(), sendblock.dataLength());
                if (b_recv > 0) {
                  blockpool->returnBlock(sendblock.data());
                  socketinfo.sendqueue.pop_front();
                }
                else {
                  socketinfo.type = FD_TCP_SSL_NEG_REDO_WRITE;
                  break;
                }
              }
            }
            else if (b_recv == 0) {
              wm->dispatchEventDisconnected(er);
              closeSocketIntern(sockid);
              break;
            }
            else {
              if (!investigateSSLError(SSL_get_error(ssl, b_recv), sockid, b_recv)) {
                wm->dispatchEventDisconnected(er);
                closeSocketIntern(sockid);
              }
            }
          }
          break;
        case FD_TCP_SSL_NEG_REDO_WRITE: // tcp ssl redo write
          if (pollevent == POLLEVENT_IN) { // incoming data
            ssl = socketinfo.ssl;
            DataBlock sendblock = socketinfo.sendqueue.front();
            b_recv = SSL_write(ssl, sendblock.data(), sendblock.dataLength());
            if (b_recv > 0) {
              blockpool->returnBlock(sendblock.data());
              socketinfo.sendqueue.pop_front();
              socketinfo.type = FD_TCP_SSL;
              while (socketinfo.sendqueue.size() > 0) {
                sendblock = socketinfo.sendqueue.front();
                b_recv = SSL_write(ssl, sendblock.data(), sendblock.dataLength());
                if (b_recv > 0) {
                  blockpool->returnBlock(sendblock.data());
                  socketinfo.sendqueue.pop_front();
                }
                else {
                  socketinfo.type = FD_TCP_SSL_NEG_REDO_WRITE;
                  break;
                }
              }
            }
            else if (b_recv == 0) {
              wm->dispatchEventDisconnected(er);
              closeSocketIntern(sockid);
            }
            else if (b_recv < 0) {
              if (!investigateSSLError(SSL_get_error(ssl, b_recv), sockid, b_recv)) {
                wm->dispatchEventDisconnected(er);
                closeSocketIntern(sockid);
              }
            }
          }
          break;
        case FD_TCP_SSL: // tcp ssl
          if (pollevent == POLLEVENT_IN) { // incoming data
            ssl = socketinfo.ssl;
            while (true) {
              buf = blockpool->getBlock();
              b_recv = SSL_read(ssl, buf, blocksize);
              if (b_recv < 0) {
                if (!investigateSSLError(SSL_get_error(ssl, b_recv), sockid, b_recv)) {
                  wm->dispatchEventDisconnected(er);
                  closeSocketIntern(sockid);
                }
                blockpool->returnBlock(buf);
                break;
              }
              else if (b_recv == 0) {
                blockpool->returnBlock(buf);
                wm->dispatchEventDisconnected(er);
                closeSocketIntern(sockid);
                break;
              }
              wm->dispatchFDData(er, buf, b_recv);
            }
          }
          break;
        case FD_UDP: // udp
          if (pollevent == POLLEVENT_IN) { // incoming data
            buf = blockpool->getBlock();
            b_recv = recvfrom(currfd, buf, blocksize, 0, (struct sockaddr *) 0, (socklen_t *) 0);
            wm->dispatchFDData(er, buf, b_recv);
          }
          break;
        case FD_TCP_SERVER: // tcp server
          if (pollevent == POLLEVENT_IN) { // incoming connection
            newfd = accept(currfd, &addr, &addrlen);
            fcntl(newfd, F_SETFL, O_NONBLOCK);
            int newsockid = sockidcounter++;
            socketinfomap[newsockid].fd = newfd;
            socketinfomap[newsockid].type = FD_TCP_PLAIN_LISTEN;
            wm->dispatchEventNew(er, newsockid);
          }
          break;
      }
    }
    if (!fds.size()) { // timeout occurred

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
