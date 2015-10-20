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
  sendblockpool(makePointer<DataBlockPool>()),
  blocksize(blockpool->blockSize()),
  sockidcounter(0),
  hasdefaultinterface(false) {

}

void IOManager::init() {
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
#ifdef _ISOC95_SOURCE
  pthread_setname_np(thread, "IO");
#endif
  global->getTickPoke()->startPoke(this, "IOManager", TICKPERIOD, 0);
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
  if (retcode) {
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
    if (retcode) {
      if (!handleError(er)) {
        return -1;
      }
    }
    retcode = bind(sockfd, res2->ai_addr, res2->ai_addrlen);
    if (retcode) {
      if (!handleError(er)) {
        return -1;
      }
    }
  }
  retcode = connect(sockfd, res->ai_addr, res->ai_addrlen);
  if (retcode) {
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
  socketinfomap[sockid].id = sockid;
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
  if (retcode) {
    if (!handleError(er)) {
      return -1;
    }
  }
  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  int yes = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  retcode = bind(sockfd, res->ai_addr, res->ai_addrlen);
  if (retcode) {
    if (!handleError(er)) {
      return -1;
    }
  }
  retcode = listen(sockfd, 10);
  if (retcode) {
    if (!handleError(er)) {
      return -1;
    }
  }
  ScopeLock lock(socketinfomaplock);
  int sockid = sockidcounter++;
  socketinfomap[sockid].fd = sockfd;
  socketinfomap[sockid].id = sockid;
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
  socketinfomap[sockid].id = sockid;
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
  if (retcode) {
    if (!handleError(er)) {
      return -1;
    }
  }
  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  int yes = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  retcode = bind(sockfd, res->ai_addr, res->ai_addrlen);
  if (retcode) {
    if (!handleError(er)) {
      return -1;
    }
  }
  ScopeLock lock(socketinfomaplock);
  int sockid = sockidcounter++;
  socketinfomap[sockid].fd = sockfd;
  socketinfomap[sockid].id = sockid;
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
    it->second.type = FD_TCP_SSL_NEG_CONNECT;
    lock.unlock();
    polling.setFDOut(it->second.fd);
  }
}

void IOManager::negotiateSSLAccept(int id) {
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::iterator it = socketinfomap.find(id);
    if (it != socketinfomap.end() &&
       (it->second.type == FD_TCP_PLAIN || it->second.type == FD_TCP_PLAIN_LISTEN))
    {
      it->second.type = FD_TCP_SSL_NEG_ACCEPT;
      polling.setFDOut(it->second.fd);
  }
}

void IOManager::forceSSLhandshake(int id) {
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::iterator it = socketinfomap.find(id);
  if (it == socketinfomap.end()) {
    return;
  }
  SocketInfo & socketinfo = it->second;
  socketinfo.type = FD_TCP_SSL_NEG_REDO_HANDSHAKE;
  polling.setFDOut(socketinfo.fd);
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
  util::assert(buflen <= MAX_SEND_BUFFER);
  unsigned int sendblocksize = sendblockpool->blockSize();
  char * datablock;
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::iterator it = socketinfomap.find(id);
  if (it == socketinfomap.end()) {
    return;
  }
  SocketInfo & socketinfo = it->second;
  while (buflen > 0) {
    datablock = sendblockpool->getBlock();
    int copysize = buflen < sendblocksize ? buflen : sendblocksize;
    memcpy(datablock, buf, copysize);
    socketinfo.sendqueue.push_back(DataBlock(datablock, copysize));
    buf += copysize;
    buflen -= copysize;
  }
  util::assert(socketinfo.sendqueue.size() * sendblockpool->blockSize() <= MAX_SEND_BUFFER);
  polling.setFDOut(socketinfo.fd);
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
  polling.removeFD(socketinfo.fd);
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

void IOManager::handleKeyboardIn(SocketInfo & socketinfo, ScopeLock & lock) {
  lock.unlock();
  wm->dispatchFDData(socketinfo.receiver);
  lock.lock();
}

void IOManager::handleTCPConnectingOut(SocketInfo & socketinfo) {
  unsigned int error;
  socklen_t errorlen = sizeof(error);
  getsockopt(socketinfo.fd, SOL_SOCKET, SO_ERROR, &error, &errorlen);
  if (error != 0) {
    wm->dispatchEventFail(socketinfo.receiver, strerror(error));
    closeSocketIntern(socketinfo.id);
    return;
  }
  socketinfo.type = FD_TCP_PLAIN;
  connecttimemap.erase(socketinfo.id);
  wm->dispatchEventConnected(socketinfo.receiver);
  polling.setFDIn(socketinfo.fd);
}

void IOManager::handleTCPPlainIn(SocketInfo & socketinfo) {
  char * buf = blockpool->getBlock();
  int b_recv = read(socketinfo.fd, buf, blocksize);
  if (b_recv == 0) {
    blockpool->returnBlock(buf);
    wm->dispatchEventDisconnected(socketinfo.receiver);
    closeSocketIntern(socketinfo.id);
    return;
  }
  else if (b_recv < 0) {
    blockpool->returnBlock(buf);
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return;
    }
    wm->dispatchEventDisconnected(socketinfo.receiver);
    global->getEventLog()->log("IOManager",
        "Socket read error on established connection to "
        + socketinfo.addr + ": " + strerror(errno));
    closeSocketIntern(socketinfo.id);
    return;
  }
  wm->dispatchFDData(socketinfo.receiver, buf, b_recv);
}

void IOManager::handleTCPPlainOut(SocketInfo & socketinfo) {
  while (socketinfo.sendqueue.size() > 0) {
    DataBlock & block = socketinfo.sendqueue.front();
    int b_sent = send(socketinfo.fd, block.data(), block.dataLength(), MSG_NOSIGNAL);
    if (b_sent < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return;
      }
      wm->dispatchEventDisconnected(socketinfo.receiver);
      global->getEventLog()->log("IOManager",
          "Socket write error on established connection to "
          + socketinfo.addr + ": " + strerror(errno));
      closeSocketIntern(socketinfo.id);
      return;
    }
    if (b_sent < block.dataLength()) {
      block.consume(b_sent);
      return;
    }
    else {
      sendblockpool->returnBlock(block.rawData());
      socketinfo.sendqueue.pop_front();
    }
  }
  if (!socketinfo.sendqueue.size()) {
    polling.setFDIn(socketinfo.fd);
    wm->dispatchEventSendComplete(socketinfo.receiver);
  }
}

void IOManager::handleTCPSSLNegotiationIn(SocketInfo & socketinfo) {
  SSL * ssl = socketinfo.ssl;
  int b_recv;
  if (socketinfo.type == FD_TCP_SSL_NEG_REDO_CONNECT) {
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
    std::string cipher = SSL_CIPHER_get_name(SSL_get_current_cipher(ssl));
    wm->dispatchEventSSLSuccess(socketinfo.receiver);
    if (socketinfo.sendqueue.size() > 0) {
      polling.setFDOut(socketinfo.fd);
    }
  }
  else if (b_recv == 0) {
    wm->dispatchEventDisconnected(socketinfo.receiver);
    closeSocketIntern(socketinfo.id);
  }
  else {
    if (!investigateSSLError(SSL_get_error(ssl, b_recv), socketinfo.id, b_recv)) {
      wm->dispatchEventDisconnected(socketinfo.receiver);
      closeSocketIntern(socketinfo.id);
    }
  }
}

void IOManager::handleTCPSSLNegotiationOut(SocketInfo & socketinfo) {
  if (socketinfo.type == FD_TCP_SSL_NEG_REDO_HANDSHAKE) {
    polling.setFDIn(socketinfo.fd);
    handleTCPSSLNegotiationIn(socketinfo);
    return;
  }
  SSL * ssl = SSL_new(global->getSSLCTX());
  SSL_set_mode(ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
  socketinfo.ssl = ssl;
  SSL_set_fd(ssl, socketinfo.fd);
  if (socketinfo.receiver != NULL) {
    std::map<int, SocketInfo>::iterator it2;
    SocketInfo * parentsocketinfo = NULL;
    for (it2 = socketinfomap.begin(); it2 != socketinfomap.end(); it2++) {
      if (it2->second.receiver == socketinfo.receiver) {
        parentsocketinfo = &it2->second;
        break;
      }
    }
    if (parentsocketinfo != NULL && parentsocketinfo->ssl != NULL) {
      SSL_copy_session_id(ssl, parentsocketinfo->ssl);
    }
  }
  int ret = -1;
  if (socketinfo.type == FD_TCP_SSL_NEG_CONNECT) {
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
    if (!investigateSSLError(SSL_get_error(ssl, ret), socketinfo.id, ret)) {
      wm->dispatchEventDisconnected(socketinfo.receiver);
      closeSocketIntern(socketinfo.id);
      return;
    }
    if (socketinfo.type == FD_TCP_SSL_NEG_CONNECT) {
      socketinfo.type = FD_TCP_SSL_NEG_REDO_CONNECT;
    }
    else {
      socketinfo.type = FD_TCP_SSL_NEG_REDO_ACCEPT;
    }
  }
  polling.setFDIn(socketinfo.fd);
}

void IOManager::handleTCPSSLIn(SocketInfo & socketinfo) {
  SSL * ssl = socketinfo.ssl;
  while (true) {
    char * buf = blockpool->getBlock();
    int b_recv = SSL_read(ssl, buf, blocksize);
    if (b_recv < 0) {
      if (!investigateSSLError(SSL_get_error(ssl, b_recv), socketinfo.id, b_recv)) {
        wm->dispatchEventDisconnected(socketinfo.receiver);
        closeSocketIntern(socketinfo.id);
      }
      blockpool->returnBlock(buf);
      return;
    }
    else if (b_recv == 0) {
      blockpool->returnBlock(buf);
      wm->dispatchEventDisconnected(socketinfo.receiver);
      closeSocketIntern(socketinfo.id);
      return;
    }
    wm->dispatchFDData(socketinfo.receiver, buf, b_recv);
  }
}

void IOManager::handleTCPSSLOut(SocketInfo & socketinfo) {
  while (socketinfo.sendqueue.size() > 0) {
    DataBlock & block = socketinfo.sendqueue.front();
    SSL * ssl = socketinfo.ssl;
    int b_sent = SSL_write(ssl, block.data(), block.dataLength());
    if (b_sent < 0) {
      int code = SSL_get_error(ssl, b_sent);
      if (code == SSL_ERROR_WANT_READ || code == SSL_ERROR_WANT_WRITE) {
        return;
      }
      else {
        wm->dispatchEventDisconnected(socketinfo.receiver);
        closeSocketIntern(socketinfo.id);
      }
      return;
    }
    sendblockpool->returnBlock(block.rawData());
    socketinfo.sendqueue.pop_front();
  }
  if (!socketinfo.sendqueue.size()) {
    polling.setFDIn(socketinfo.fd);
    wm->dispatchEventSendComplete(socketinfo.receiver);
  }
}

void IOManager::handleUDPIn(SocketInfo & socketinfo) {
  char * buf = blockpool->getBlock();
  int b_recv = recvfrom(socketinfo.fd, buf, blocksize, 0, (struct sockaddr *) 0, (socklen_t *) 0);
  wm->dispatchFDData(socketinfo.receiver, buf, b_recv);
}

void IOManager::handleTCPServerIn(SocketInfo & socketinfo) {
  struct sockaddr addr;
  socklen_t addrlen;
  int newfd = accept(socketinfo.fd, &addr, &addrlen);
  fcntl(newfd, F_SETFL, O_NONBLOCK);
  int newsockid = sockidcounter++;
  socketinfomap[newsockid].fd = newfd;
  socketinfomap[newsockid].id = newsockid;
  socketinfomap[newsockid].type = FD_TCP_PLAIN_LISTEN;
  wm->dispatchEventNew(socketinfo.receiver, newsockid);
}

void IOManager::runInstance() {
  std::list<std::pair<int, PollEvent> > fds;
  std::list<std::pair<int, PollEvent> >::const_iterator pollit;
  ScopeLock lock(socketinfomaplock);
  while(1) {
    lock.unlock();
    blockpool->awaitFreeBlocks();
    polling.wait(fds);
    lock.lock();
    for (pollit = fds.begin(); pollit != fds.end(); pollit++) {
      int currfd = pollit->first;
      PollEvent pollevent = pollit->second;
      std::map<int, int>::iterator idit = sockfdidmap.find(currfd);
      if (idit == sockfdidmap.end()) {
        continue;
      }
      int sockid = idit->second;
      std::map<int, SocketInfo>::iterator it = socketinfomap.find(sockid);
      if (it == socketinfomap.end()) {
        continue;
      }
      SocketInfo & socketinfo = it->second;
      switch (socketinfo.type) {
        case FD_KEYBOARD: // keyboard
          handleKeyboardIn(socketinfo, lock);
          break;
        case FD_TCP_CONNECTING:
          if (pollevent == POLLEVENT_OUT) {
            handleTCPConnectingOut(socketinfo);
          }
          break;
        case FD_TCP_PLAIN: // tcp plain
        case FD_TCP_PLAIN_LISTEN:
          if (pollevent == POLLEVENT_IN) { // incoming data
            handleTCPPlainIn(socketinfo);
          }
          else if (pollevent == POLLEVENT_OUT) {
            handleTCPPlainOut(socketinfo);
          }
          break;
        case FD_TCP_SSL_NEG_CONNECT: // tcp ssl connect
        case FD_TCP_SSL_NEG_REDO_CONNECT: // tcp ssl redo connect
        case FD_TCP_SSL_NEG_ACCEPT: // tcp ssl accept
        case FD_TCP_SSL_NEG_REDO_ACCEPT: // tcp ssl redo accept
        case FD_TCP_SSL_NEG_REDO_HANDSHAKE: // tcp ssl redo handshake
          if (pollevent == POLLEVENT_IN) { // incoming data
            handleTCPSSLNegotiationIn(socketinfo);
          }
          else if (pollevent == POLLEVENT_OUT) {
            handleTCPSSLNegotiationOut(socketinfo);
          }
          break;
        case FD_TCP_SSL: // tcp ssl
          if (pollevent == POLLEVENT_IN) { // incoming data
            handleTCPSSLIn(socketinfo);
          }
          else if (pollevent == POLLEVENT_OUT) {
            handleTCPSSLOut(socketinfo);
          }
          break;
        case FD_UDP: // udp
          if (pollevent == POLLEVENT_IN) { // incoming data
            handleUDPIn(socketinfo);
          }
          break;
        case FD_TCP_SERVER: // tcp server
          if (pollevent == POLLEVENT_IN) { // incoming connection
            handleTCPServerIn(socketinfo);
          }
          break;
        case FD_UNUSED:
          util::assert(false);
          break;
      }
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
