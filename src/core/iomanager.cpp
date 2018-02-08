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
#include <openssl/err.h>
#include <vector>
#include <cerrno>
#include <csignal>
#include <cstring>

#include "workmanager.h"
#include "sslmanager.h"
#include "datablock.h"
#include "datablockpool.h"
#include "logger.h"
#include "tickpoke.h"
#include "scopelock.h"
#include "util.h"

#define TICKPERIOD 100
#define TIMEOUT_MS 5000
#define MAX_SEND_BUFFER 1048576

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#ifdef SO_NOSIGPIPE
#define USE_SO_NOSIGPIPE
#else
#error "Cannot block SIGPIPE!"
#endif
#endif

namespace {

enum AsyncTaskTypes {
  ASYNC_DNS_RESOLUTION
};

void resolveDNSAsync(EventReceiver * er, int sockid) {
  static_cast<IOManager *>(er)->resolveDNS(sockid);
}

bool needsDNSResolution(const std::string & addr) {
  size_t addrlen = addr.length();
  if (addrlen >= 4) {
    if (isalpha(addr[addrlen - 1]) && isalpha(addr[addrlen - 2])) {
      if (addr[addrlen - 3] == '.' || (isalpha(addr[addrlen - 3]) && addr[addrlen - 4] == '.')) {
        return true;
      }
    }
  }
  return false;
}

}

IOManager::IOManager(WorkManager * wm, TickPoke * tp) :
  wm(wm),
  tp(tp),
  blockpool(wm->getBlockPool()),
  sendblockpool(makePointer<DataBlockPool>()),
  blocksize(blockpool->blockSize()),
  sockidcounter(0),
  hasdefaultinterface(false)
{
  wm->addReadyNotify(this);
}

void IOManager::init() {
  SSLManager::init();
  thread.start("IO", this);
  tp->startPoke(this, "IOManager", TICKPERIOD, 0);
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
            wm->dispatchEventFail(er, it2->second.id, "Connection timeout");
          }
          closeSocketIntern(it->first);
          break;
        }
      }
    }
  }
}

void IOManager::pollRead(SocketInfo & socketinfo) {
  socketinfo.direction = DIR_IN;
  if (!socketinfo.paused) {
    polling.addFDIn(socketinfo.fd);
  }
}

void IOManager::pollWrite(SocketInfo & socketinfo) {
  socketinfo.direction = DIR_OUT;
  if (!socketinfo.paused) {
    polling.addFDOut(socketinfo.fd);
  }
}

void IOManager::setPollRead(SocketInfo & socketinfo) {
  socketinfo.direction = DIR_IN;
  if (!socketinfo.paused) {
    polling.setFDIn(socketinfo.fd);
  }
}

void IOManager::setPollWrite(SocketInfo & socketinfo) {
  socketinfo.direction = DIR_OUT;
  if (!socketinfo.paused) {
    polling.setFDOut(socketinfo.fd);
  }
}

void IOManager::unsetPoll(SocketInfo & socketinfo) {
  polling.removeFD(socketinfo.fd);
}

int IOManager::registerTCPClientSocket(EventReceiver * er, const std::string & addr, int port) {
  bool resolving;
  return registerTCPClientSocket(er, addr, port, resolving, true);
}

int IOManager::registerTCPClientSocket(EventReceiver * er, const std::string & addr, int port, bool & resolving, bool listenimmediately) {
  socketinfomaplock.lock();
  int sockid = sockidcounter++;
  SocketInfo & socketinfo = socketinfomap[sockid];
  socketinfo.id = sockid;
  socketinfo.fd = -1;
  socketinfo.addr = addr;
  socketinfo.port = port;
  socketinfo.type = FD_TCP_RESOLVING;
  socketinfo.receiver = er;
  socketinfo.gaiasync = resolving = needsDNSResolution(addr);
  socketinfo.listenimmediately = listenimmediately;
  connecttimemap[sockid] = 0;
  socketinfomaplock.unlock();
  if (resolving) {
    wm->asyncTask(this, ASYNC_DNS_RESOLUTION, &resolveDNSAsync, sockid);
  }
  else
  {
    resolveDNS(sockid);
    ScopeLock lock(socketinfomaplock);
    handleTCPNameResolution(socketinfo);
  }
  return sockid;
}

void IOManager::handleTCPNameResolution(SocketInfo & socketinfo) {
  if (socketinfo.gairet) {
    wm->dispatchEventFail(socketinfo.receiver, -1, socketinfo.gaierr);
    closeSocketIntern(socketinfo.id);
    return;
  }
  struct addrinfo * result = socketinfo.gaires;
  int sockfd = socket(result->ai_family,
                      result->ai_socktype,
                      result->ai_protocol);
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
#ifdef USE_SO_NOSIGPIPE
  int yes = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(yes));
#endif
  char * buf = (char *) malloc(result->ai_addrlen);
  struct sockaddr_in * saddr = (struct sockaddr_in*) result->ai_addr;
  inet_ntop(AF_INET, &(saddr->sin_addr), buf, result->ai_addrlen);
  std::string addrstr = buf;
  free(buf);
  socketinfo.fd = sockfd;
  socketinfo.type = FD_TCP_CONNECTING;
  socketinfo.addr = addrstr;
  connecttimemap[socketinfo.id] = 0;
  sockfdidmap[sockfd] = socketinfo.id;
  if (hasDefaultInterface()) {
    struct addrinfo request, *res;
    memset(&request, 0, sizeof(request));
    request.ai_family = AF_INET;
    request.ai_socktype = SOCK_STREAM;
    int retcode = getaddrinfo(getInterfaceAddress(getDefaultInterface()).c_str(),
                              "0", &request, &res);
    if (retcode) {
      if (!handleError(socketinfo.receiver)) {
        closeSocketIntern(socketinfo.id);
        return;
      }
    }
    retcode = bind(sockfd, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);
    if (retcode) {
      if (!handleError(socketinfo.receiver)) {
        closeSocketIntern(socketinfo.id);
        return;
      }
    }
  }
  int retcode = connect(sockfd, result->ai_addr, result->ai_addrlen);
  if (retcode) {
    if (!handleError(socketinfo.receiver)) {
      closeSocketIntern(socketinfo.id);
      return;
    }
  }

  if (socketinfo.gaiasync) {
    wm->dispatchEventConnecting(socketinfo.receiver, socketinfo.id, socketinfo.addr);
  }
  freeaddrinfo(result);
  socketinfo.gaires = NULL;
  pollWrite(socketinfo);
  return;
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
  int retcode = getaddrinfo(addr.c_str(), coreutil::int2Str(port).c_str(), &sock, &res);
  if (retcode) {
    if (!handleError(er)) {
      return -1;
    }
  }
  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  int yes = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#ifdef USE_SO_NOSIGPIPE
  setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(yes));
#endif
  retcode = bind(sockfd, res->ai_addr, res->ai_addrlen);
  freeaddrinfo(res);
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
  SocketInfo & socketinfo = socketinfomap[sockid];
  socketinfo.fd = sockfd;
  socketinfo.id = sockid;
  socketinfo.type = FD_TCP_SERVER;
  socketinfo.receiver = er;
  sockfdidmap[sockfd] = sockid;
  pollRead(socketinfo);
  return sockid;
}

void IOManager::registerTCPServerClientSocket(EventReceiver * er, int sockid) {
  registerTCPServerClientSocket(er, sockid, true);
}

void IOManager::registerTCPServerClientSocket(EventReceiver * er, int sockid, bool listenimmediately) {
  ScopeLock lock(socketinfomaplock);
  socketinfomap[sockid].receiver = er;
  socketinfomap[sockid].listenimmediately = listenimmediately;
  if (listenimmediately) {
    pollRead(socketinfomap[sockid]);
  }
}

void IOManager::registerStdin(EventReceiver * er) {
  ScopeLock lock(socketinfomaplock);
  int sockid = sockidcounter++;
  SocketInfo & socketinfo = socketinfomap[sockid];
  socketinfo.fd = STDIN_FILENO;
  socketinfo.id = sockid;
  socketinfo.type = FD_KEYBOARD;
  socketinfo.receiver = er;
  sockfdidmap[STDIN_FILENO] = sockid;
  pollRead(socketinfo);
}

int IOManager::registerUDPServerSocket(EventReceiver * er, int port) {
  struct addrinfo sock, *res;
  memset(&sock, 0, sizeof(sock));
  sock.ai_family = AF_UNSPEC;
  sock.ai_socktype = SOCK_DGRAM;
  sock.ai_protocol = IPPROTO_UDP;
  std::string addr = "0.0.0.0";
  int retcode = getaddrinfo(addr.c_str(), coreutil::int2Str(port).c_str(), &sock, &res);
  if (retcode) {
    if (!handleError(er)) {
      return -1;
    }
  }
  int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  fcntl(sockfd, F_SETFL, O_NONBLOCK);
  int yes = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
#ifdef USE_SO_NOSIGPIPE
  setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(yes));
#endif
  retcode = bind(sockfd, res->ai_addr, res->ai_addrlen);
  freeaddrinfo(res);
  if (retcode) {
    if (!handleError(er)) {
      return -1;
    }
  }
  ScopeLock lock(socketinfomaplock);
  int sockid = sockidcounter++;
  SocketInfo & socketinfo = socketinfomap[sockid];
  socketinfo.fd = sockfd;
  socketinfo.id = sockid;
  socketinfo.type = FD_UDP;
  socketinfo.receiver = er;
  sockfdidmap[sockfd] = sockid;
  pollRead(socketinfo);
  return sockid;
}

void IOManager::adopt(EventReceiver * er, int id) {
  ScopeLock lock(socketinfomaplock);
  socketinfomap[id].receiver = er;
}

void IOManager::negotiateSSLConnect(int id) {
  negotiateSSLConnect(id, -1);
}

void IOManager::negotiateSSLConnect(int id, int parentid) {
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::iterator it = socketinfomap.find(id);
  if (it != socketinfomap.end()) {
    it->second.parentid = parentid;
    if (it->second.type == FD_TCP_PLAIN || it->second.type == FD_TCP_PLAIN_LISTEN) {
      it->second.type = FD_TCP_SSL_NEG_CONNECT;
      if (it->second.listenimmediately) {
        setPollWrite(it->second);
      }
      else {
        pollWrite(it->second);
      }
    }
  }
}

void IOManager::negotiateSSLAccept(int id) {
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::iterator it = socketinfomap.find(id);
    if (it != socketinfomap.end() &&
       (it->second.type == FD_TCP_PLAIN || it->second.type == FD_TCP_PLAIN_LISTEN))
    {
      it->second.type = FD_TCP_SSL_NEG_ACCEPT;
      if (it->second.listenimmediately) {
        setPollWrite(it->second);
      }
      else {
        pollWrite(it->second);
      }
  }
}

bool IOManager::handleError(EventReceiver * er) {
  if (errno == EINPROGRESS) {
    return true;
  }
  wm->dispatchEventFail(er, -1, strerror(errno));
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
        setPollWrite(socketinfo);
        return true;
      }
      break;
  }
  unsigned long e = ERR_get_error();
  log("SSL error on connection to " +
      it->second.addr + ": " +
      coreutil::int2Str(error) + " return code: " + coreutil::int2Str(b_recv) +
      " errno: " + strerror(errno) +
      (e ? " String: " + std::string(ERR_error_string(e, NULL)) : ""));
  return false;
}

bool IOManager::sendData(int id, const std::string & data) {
  char * buf = (char *) data.c_str();
  return sendData(id, buf, data.length());
}

bool IOManager::sendData(int id, const char * buf, unsigned int buflen) {
  coreutil::assert(buflen <= MAX_SEND_BUFFER);
  unsigned int sendblocksize = sendblockpool->blockSize();
  char * datablock;
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::iterator it = socketinfomap.find(id);
  if (it == socketinfomap.end()) {
    return true;
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
  if (socketinfo.type == FD_TCP_PLAIN || socketinfo.type == FD_TCP_PLAIN_LISTEN ||
      socketinfo.type == FD_TCP_SSL)
  {
    setPollWrite(socketinfo);
  }
  return socketinfo.sendqueue.size() * sendblockpool->blockSize() <= MAX_SEND_BUFFER;
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
  if (socketinfo.ssl) {
    int shutdownstate = SSL_get_shutdown(socketinfo.ssl);
    if (!(shutdownstate & SSL_SENT_SHUTDOWN)) {
      if (!(shutdownstate & SSL_RECEIVED_SHUTDOWN)) {
        SSL_shutdown(socketinfo.ssl);
      }
      else {
        SSL_set_shutdown(socketinfo.ssl, SSL_SENT_SHUTDOWN | shutdownstate);
      }
    }
  }
  if (socketinfo.fd > 0) {
    close(socketinfo.fd);
  }
  if (socketinfo.ssl) {
    SSL_free(socketinfo.ssl);
  }
  if (socketinfo.gaires) {
    freeaddrinfo(static_cast<struct addrinfo*>(socketinfo.gaires));
  }
  for (std::list<DataBlock>::iterator it = socketinfo.sendqueue.begin(); it != socketinfo.sendqueue.end(); it++) {
    sendblockpool->returnBlock(it->rawData());
  }
  socketinfo.sendqueue.clear();
  sockfdidmap.erase(socketinfo.fd);
  connecttimemap.erase(id);
  autopaused.erase(id);
  manuallypaused.erase(id);
  socketinfomap.erase(it);
}

void IOManager::resolveDNS(int id) {
  socketinfomaplock.lock();
  std::map<int, SocketInfo>::iterator it = socketinfomap.find(id);
  if (it == socketinfomap.end()) {
    socketinfomaplock.unlock();
    return;
  }
  std::string addr = it->second.addr;
  int port = it->second.port;
  socketinfomaplock.unlock();
  struct addrinfo request;
  memset(&request, 0, sizeof(request));
  request.ai_family = AF_INET;
  request.ai_socktype = SOCK_STREAM;
  struct addrinfo * result;
  int retcode = getaddrinfo(addr.c_str(), coreutil::int2Str(port).c_str(), &request, &result);
  ScopeLock lock(socketinfomaplock);
  it = socketinfomap.find(id);
  if (it == socketinfomap.end()) {
    if (!retcode) {
      freeaddrinfo(result);
    }
    return;
  }
  it->second.gairet = retcode;
  if (retcode) {
    it->second.gaierr = gai_strerror(retcode);
  }
  else {
    it->second.gaires = result;
  }
}

void IOManager::asyncTaskComplete(int type, int id) {
  coreutil::assert(type == ASYNC_DNS_RESOLUTION);
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::iterator it = socketinfomap.find(id);
  if (it == socketinfomap.end()) {
    return;
  }
  coreutil::assert(it->second.type == FD_TCP_RESOLVING);
  handleTCPNameResolution(it->second);
}

std::string IOManager::getCipher(int id) const {
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::const_iterator it = socketinfomap.find(id);
  if (it == socketinfomap.end() || it->second.ssl == NULL || it->second.type != FD_TCP_SSL) {
    return "";
  }
  const char * cipher = SSLManager::getCipher(it->second.ssl);
  return std::string(cipher);
}

bool IOManager::getSSLSessionReused(int id) const {
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::const_iterator it = socketinfomap.find(id);
  if (it == socketinfomap.end() || it->second.ssl == NULL || it->second.type != FD_TCP_SSL) {
    return false;
  }
  return SSL_session_reused(it->second.ssl);
}

std::string IOManager::getSocketAddress(int id) const {
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::const_iterator it = socketinfomap.find(id);
  if (it != socketinfomap.end()) {
    return it->second.addr;
  }
  return "";
}

int IOManager::getSocketPort(int id) const {
  std::string addr = "";
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::const_iterator it = socketinfomap.find(id);
  if (it != socketinfomap.end()) {
    return it->second.port;
  }
  return -1;
}

std::string IOManager::getInterfaceAddress(int id) const {
  std::string addr = "";
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::const_iterator it = socketinfomap.find(id);
  if (it != socketinfomap.end()) {
    addr = it->second.localaddr;
  }
  return addr;
}

void IOManager::handleKeyboardIn(SocketInfo & socketinfo, ScopeLock & lock) {
  lock.unlock();
  wm->dispatchFDData(socketinfo.receiver, socketinfo.id);
  lock.lock();
}

void IOManager::handleTCPConnectingOut(SocketInfo & socketinfo) {
  unsigned int error;
  socklen_t errorlen = sizeof(error);
  getsockopt(socketinfo.fd, SOL_SOCKET, SO_ERROR, &error, &errorlen);
  if (error != 0) {
    wm->dispatchEventFail(socketinfo.receiver, socketinfo.id, strerror(error));
    closeSocketIntern(socketinfo.id);
    return;
  }
  struct sockaddr_in localaddr;
  socklen_t localaddrlen = sizeof(localaddr);
  char * buf = (char *) malloc(localaddrlen);
  getsockname(socketinfo.fd, (struct sockaddr *)&localaddr, &localaddrlen);
  inet_ntop(AF_INET, &localaddr.sin_addr, buf, localaddrlen);
  std::string localaddrstr = buf;
  free(buf);
  socketinfo.type = FD_TCP_PLAIN;
  socketinfo.localaddr = localaddrstr;
  socketinfo.localport = ntohs(localaddr.sin_port);
  connecttimemap.erase(socketinfo.id);
  wm->dispatchEventConnected(socketinfo.receiver, socketinfo.id);
  if (socketinfo.listenimmediately) {
    if (!socketinfo.sendqueue.size()) {
      setPollRead(socketinfo);
    }
  }
  else {
    unsetPoll(socketinfo);
  }
}

void IOManager::handleTCPPlainIn(SocketInfo & socketinfo) {
  char * buf = blockpool->getBlock();
  int b_recv = read(socketinfo.fd, buf, blocksize);
  if (b_recv == 0) {
    blockpool->returnBlock(buf);
    wm->dispatchEventDisconnected(socketinfo.receiver, socketinfo.id, socketinfo.prio);
    closeSocketIntern(socketinfo.id);
    return;
  }
  else if (b_recv < 0) {
    blockpool->returnBlock(buf);
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return;
    }
    wm->dispatchEventDisconnected(socketinfo.receiver, socketinfo.id, socketinfo.prio);
    log("Socket read error on established connection to "
        + socketinfo.addr + ": " + strerror(errno));
    closeSocketIntern(socketinfo.id);
    return;
  }
  if (!wm->dispatchFDData(socketinfo.receiver, socketinfo.id, buf, b_recv, socketinfo.prio)) {
    autoPause(socketinfo);
  }
}

void IOManager::handleTCPPlainOut(SocketInfo & socketinfo) {
  while (socketinfo.sendqueue.size() > 0) {
    DataBlock & block = socketinfo.sendqueue.front();
    int b_sent = send(socketinfo.fd, block.data(), block.dataLength(), MSG_NOSIGNAL);
    if (b_sent < 0) {
      if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return;
      }
      wm->dispatchEventDisconnected(socketinfo.receiver, socketinfo.id, socketinfo.prio);
      log("Socket write error on established connection to "
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
    setPollRead(socketinfo);
    wm->dispatchEventSendComplete(socketinfo.receiver, socketinfo.id, socketinfo.prio);
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
    const char * cipher = SSLManager::getCipher(ssl);
    wm->dispatchEventSSLSuccess(socketinfo.receiver, socketinfo.id, cipher, socketinfo.prio);
    if (socketinfo.sendqueue.size() > 0) {
      setPollWrite(socketinfo);
    }
  }
  else if (b_recv == 0) {
    wm->dispatchEventDisconnected(socketinfo.receiver, socketinfo.id, socketinfo.prio);
    closeSocketIntern(socketinfo.id);
  }
  else {
    if (!investigateSSLError(SSL_get_error(ssl, b_recv), socketinfo.id, b_recv)) {
      wm->dispatchEventDisconnected(socketinfo.receiver, socketinfo.id, socketinfo.prio);
      closeSocketIntern(socketinfo.id);
    }
  }
}

void IOManager::handleTCPSSLNegotiationOut(SocketInfo & socketinfo) {
  if (socketinfo.type == FD_TCP_SSL_NEG_REDO_HANDSHAKE) {
    setPollRead(socketinfo);
    handleTCPSSLNegotiationIn(socketinfo);
    return;
  }
  if (socketinfo.type == FD_TCP_SSL_NEG_ACCEPT) {
    SSLManager::checkCertificateReady();
  }
  SSL * ssl = SSL_new(SSLManager::getSSLCTX());
  SSL_set_mode(ssl, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
  socketinfo.ssl = ssl;
  SSL_set_fd(ssl, socketinfo.fd);
  if (socketinfo.parentid != -1) {
    std::map<int, SocketInfo>::iterator it2 = socketinfomap.find(socketinfo.parentid);
    if (it2 != socketinfomap.end() && it2->second.ssl != NULL) {
      SSL_copy_session_id(ssl, it2->second.ssl);
    }
  }
  int ret = -1;
  if (socketinfo.type == FD_TCP_SSL_NEG_CONNECT) {
    ret = SSL_connect(ssl);
  }
  else if (socketinfo.type == FD_TCP_SSL_NEG_ACCEPT){
    ret = SSL_accept(ssl);
  }
  if (ret > 0) { // probably wont happen :)
    socketinfo.type = FD_TCP_SSL;
    const char * cipher = SSLManager::getCipher(ssl);
    wm->dispatchEventSSLSuccess(socketinfo.receiver, socketinfo.id, cipher, socketinfo.prio);
  }
  else {
    if (!investigateSSLError(SSL_get_error(ssl, ret), socketinfo.id, ret)) {
      wm->dispatchEventDisconnected(socketinfo.receiver, socketinfo.id, socketinfo.prio);
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
  setPollRead(socketinfo);
}

void IOManager::handleTCPSSLIn(SocketInfo & socketinfo) {
  SSL * ssl = socketinfo.ssl;
  int blocknum = 0;
  bool pause = false;
  while (true) {
    char * buf = blockpool->getBlock();
    int bufpos = 0;
    while (bufpos < blocksize) {
      int b_recv = SSL_read(ssl, buf + bufpos, blocksize - bufpos);
      if (b_recv <= 0) {
        if (bufpos > 0) {
          if (!wm->dispatchFDData(socketinfo.receiver, socketinfo.id, buf, bufpos, socketinfo.prio)) {
            autoPause(socketinfo);
          }
        }
        else {
          blockpool->returnBlock(buf);
        }
        if (!b_recv || !investigateSSLError(SSL_get_error(ssl, b_recv), socketinfo.id, b_recv)) {
          wm->dispatchEventDisconnected(socketinfo.receiver, socketinfo.id, socketinfo.prio);
          closeSocketIntern(socketinfo.id);
        }
        return;
      }
      bufpos += b_recv;
    }
    if (blocknum++ > 16) {
      socketinfo.prio = PRIO_LOW;
    }
    if (!wm->dispatchFDData(socketinfo.receiver, socketinfo.id, buf, bufpos, socketinfo.prio)) {
      pause = true;
    }
  }
  if (pause) {
    autoPause(socketinfo);
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
        wm->dispatchEventDisconnected(socketinfo.receiver, socketinfo.id, socketinfo.prio);
        closeSocketIntern(socketinfo.id);
      }
      return;
    }
    if (b_sent < block.dataLength()) {
      block.consume(b_sent);
      return;
    }
    sendblockpool->returnBlock(block.rawData());
    socketinfo.sendqueue.pop_front();
  }
  if (!socketinfo.sendqueue.size()) {
    setPollRead(socketinfo);
    wm->dispatchEventSendComplete(socketinfo.receiver, socketinfo.id, socketinfo.prio);
  }
}

void IOManager::handleUDPIn(SocketInfo & socketinfo) {
  char * buf = blockpool->getBlock();
  int b_recv = recvfrom(socketinfo.fd, buf, blocksize, 0, (struct sockaddr *) 0, (socklen_t *) 0);
  wm->dispatchFDData(socketinfo.receiver, socketinfo.id, buf, b_recv, socketinfo.prio);
}

void IOManager::handleTCPServerIn(SocketInfo & socketinfo) {
  struct sockaddr_in addr, localaddr;
  socklen_t addrlen = sizeof(addr);
  socklen_t localaddrlen = sizeof(localaddr);
  int newfd = accept(socketinfo.fd, (struct sockaddr *)&addr, &addrlen);
  char * buf = (char *) malloc(addrlen);
  inet_ntop(AF_INET, &addr.sin_addr, buf, addrlen);
  std::string addrstr = buf;
  getsockname(newfd, (struct sockaddr *)&localaddr, &localaddrlen);
  inet_ntop(AF_INET, &localaddr.sin_addr, buf, localaddrlen);
  std::string localaddrstr = buf;
  free(buf);
  fcntl(newfd, F_SETFL, O_NONBLOCK);
#ifdef USE_SO_NOSIGPIPE
  int yes = 1;
  setsockopt(newfd, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(yes));
#endif
  int newsockid = sockidcounter++;
  socketinfomap[newsockid].fd = newfd;
  socketinfomap[newsockid].id = newsockid;
  socketinfomap[newsockid].type = FD_TCP_PLAIN_LISTEN;
  socketinfomap[newsockid].addr = addrstr;
  socketinfomap[newsockid].port = ntohs(addr.sin_port);
  socketinfomap[newsockid].localaddr = localaddrstr;
  socketinfomap[newsockid].localport = ntohs(localaddr.sin_port);
  sockfdidmap[newfd] = newsockid;
  wm->dispatchEventNew(socketinfo.receiver, newsockid);
}

void IOManager::run() {
  std::list<std::pair<int, PollEvent> > fds;
  std::list<std::pair<int, PollEvent> >::const_iterator pollit;
  ScopeLock lock(socketinfomaplock);
  while (true) {
    lock.unlock();
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
        case FD_TCP_RESOLVING:
        case FD_UNUSED:
          coreutil::assert(false);
          break;
      }
    }
  }
}

std::list<std::pair<std::string, std::string> > IOManager::listInterfaces() {
  std::list<std::pair<std::string, std::string> > addrs;
  struct ifaddrs *ifaddr, *ifa;
  int family, s;
  char host[NI_MAXHOST];
  if (getifaddrs(&ifaddr) == -1) {
    log("ERROR: Failed to list network interfaces");
    return addrs;
  }
  for (ifa = ifaddr; ifa != NULL && ifa->ifa_addr != NULL; ifa = ifa->ifa_next) {
    family = ifa->ifa_addr->sa_family;
    if (family == AF_INET) {
      s = getnameinfo(ifa->ifa_addr,sizeof(struct sockaddr_in),
          host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
      if (s != 0) {
        log("ERROR: getnameinfo() failed");
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

void IOManager::setDefaultInterface(const std::string & interface) {
  if (getInterfaceAddress(interface) == "") {
    if (hasdefaultinterface) {
      hasdefaultinterface = false;
      log("Default network interface removed");
    }
  }
  else {
    if (hasdefaultinterface == false || defaultinterface != interface) {
      defaultinterface = interface;
      hasdefaultinterface = true;
      log("Default network interface set to: " + interface);
    }
  }
}

bool IOManager::hasDefaultInterface() const {
  return hasdefaultinterface;
}

std::string IOManager::getInterfaceAddress(const std::string & interface) {
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

void IOManager::setLogger(Pointer<Logger> logger) {
  this->logger = logger;
}

void IOManager::log(const std::string & text) {
  if (!!logger) {
    logger->log("IOManager", text);
  }
}

void IOManager::workerReady() {
  ScopeLock lock(socketinfomaplock);
  for (std::set<int>::iterator it = autopaused.begin(); it != autopaused.end(); it++) {
    std::map<int, SocketInfo>::iterator siit = socketinfomap.find(*it);
    if (siit == socketinfomap.end()) {
      continue;
    }
    if (manuallypaused.find(*it) == manuallypaused.end()) {
      siit->second.paused = false;
      if (siit->second.direction == DIR_IN) {
        polling.addFDIn(siit->second.fd);
      }
      else {
        polling.addFDOut(siit->second.fd);
      }
    }
  }
  autopaused.clear();
}

void IOManager::autoPause(SocketInfo & socketinfo) {
  autopaused.insert(socketinfo.id);
  socketinfo.paused = true;
  polling.removeFD(socketinfo.fd);
}

void IOManager::pause(int id) {
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::iterator it = socketinfomap.find(id);
  if (it == socketinfomap.end()) {
    return;
  }
  if (manuallypaused.find(id) == manuallypaused.end()) {
    manuallypaused.insert(id);
    it->second.paused = true;
    polling.removeFD(it->second.fd);
  }
}

void IOManager::resume(int id) {
  ScopeLock lock(socketinfomaplock);
  std::map<int, SocketInfo>::iterator it = socketinfomap.find(id);
  if (it == socketinfomap.end()) {
    return;
  }
  if (manuallypaused.find(id) != manuallypaused.end()) {
    manuallypaused.erase(id);
    if (autopaused.find(id) == autopaused.end()) {
      it->second.paused = false;
      if (it->second.direction == DIR_IN) {
        polling.addFDIn(it->second.fd);
      }
      else {
        polling.addFDOut(it->second.fd);
      }
    }
  }
}
