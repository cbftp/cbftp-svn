#include "ftpconnect.h"

#include <cassert>

#include "core/iomanager.h"
#include "site.h"
#include "ftpconnectowner.h"
#include "ftpconn.h"
#include "globalcontext.h"
#include "proxysession.h"
#include "proxy.h"

#define WELCOME_TIMEOUT_MSEC 7000

FTPConnect::FTPConnect(int id, FTPConnectOwner * owner, const Address& addr, Proxy * proxy, bool primary, bool implicittls) :
  id(id),
  sockid(-1),
  proxynegotiation(false),
  proxysession(new ProxySession()),
  owner(owner),
  databuflen(DATA_BUF_SIZE),
  databuf((char *) malloc(databuflen)),
  databufpos(0),
  addr(addr),
  resolvedaddr(addr),
  proxy(proxy),
  primary(primary),
  engaged(true),
  connected(false),
  welcomereceived(false),
  millisecs(0),
  implicittls(implicittls)
{
  bool resolving;
  if (proxy == NULL) {
    sockid = global->getIOManager()->registerTCPClientSocket(this, addr.host, addr.port, resolving, addr.addrfam);
    if (resolving) {
      owner->ftpConnectInfo(id, "[" + addr.toString() + "][Resolving]");
    }
    else {
      printConnecting(addr);
    }
  }
  else {
    proxynegotiation = true;
    proxysession->prepare(proxy, addr.host, std::to_string(addr.port));
    sockid = global->getIOManager()->registerTCPClientSocket(this, proxy->getAddr(), std::stoi(proxy->getPort()), resolving);
    if (resolving) {
      owner->ftpConnectInfo(id, "[" + addr.toString() + "][Resolving proxy " + proxy->getAddr() + "]");
    }
    else {
      Address proxyaddr;
      proxyaddr.host = proxy->getAddr();
      proxyaddr.port = std::stoi(proxy->getPort());
      printConnecting(proxyaddr);
    }
  }
}

FTPConnect::~FTPConnect() {
  assert(!engaged); // must disengage before deleting; events may still be in the work queue
  delete proxysession;
  free(databuf);
}

void FTPConnect::FDConnecting(int sockid, const std::string& addr) {
  resolvedaddr.host = addr;
  resolvedaddr.brackets = addr.find(":") != std::string::npos && resolvedaddr.port != 21;
  printConnecting(resolvedaddr, true);

}

void FTPConnect::printConnecting(const Address& addr, bool resolved) {
  if (!engaged) {
    return;
  }
  std::string out = "[" + this->addr.toString() + "][Connecting";
  if (proxynegotiation) {
    out += " to proxy " + addr.toString() + "]";
  }
  else if (resolved) {
    out += " to " + addr.toString(false) + "]";
  }
  else {
    out += "]";
  }
  owner->ftpConnectInfo(id, out);
}

void FTPConnect::FDConnected(int sockid) {
  if (!engaged) {
    return;
  }
  connected = true;
  millisecs = 0;
  owner->ftpConnectInfo(id, "[" + addr.toString() + "][Established]");
  if (proxynegotiation) {
    proxySessionInit();
  }
  else if (implicittls) {
    global->getIOManager()->negotiateSSLConnect(sockid);
  }
}

void FTPConnect::FDDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) {
  FDFail(sockid, "Disconnected: " + details);
}

void FTPConnect::FDData(int sockid, char* data, unsigned int datalen) {
  if (!engaged) {
    return;
  }
  if (proxynegotiation) {
    proxysession->received(data, datalen);
    proxySessionInit();
  }
  else {
    owner->ftpConnectInfo(id, std::string(data, datalen));
    if (FTPConn::parseData(data, datalen, &databuf, databuflen, databufpos, databufcode)) {
      if (databufcode == 220) {
        welcomereceived = true;
        owner->ftpConnectSuccess(id, resolvedaddr);
      }
      else {
        owner->ftpConnectInfo(id, "[" + addr.toString() + "][Unknown response]");
        disengage();
        owner->ftpConnectInfo(id, "[" + addr.toString() + "][Disconnected]");
        owner->ftpConnectFail(id);
      }
    }
  }
}

void FTPConnect::FDFail(int sockid, const std::string& error) {
  if (engaged) {
    engaged = false;
    owner->ftpConnectInfo(id, "[" + addr.toString() + "][" + error + "]");
    owner->ftpConnectFail(id);
  }
}

void FTPConnect::FDSSLSuccess(int sockid, const std::string& cipher) {
  owner->ftpConnectInfo(id, "[Cipher: " + cipher + "]");
}

int FTPConnect::getId() const {
  return id;
}

int FTPConnect::handedOver() {
  engaged = false;
  return sockid;
}

void FTPConnect::proxySessionInit() {
  switch (proxysession->instruction()) {
    case PROXYSESSION_SEND_CONNECT:
      owner->ftpConnectInfo(id, "[" + addr.toString() + "][Connecting through proxy]");
      global->getIOManager()->sendData(sockid, proxysession->getSendData(), proxysession->getSendDataLen());
      break;
    case PROXYSESSION_SEND:
      global->getIOManager()->sendData(sockid, proxysession->getSendData(), proxysession->getSendDataLen());
      break;
    case PROXYSESSION_SUCCESS:
      owner->ftpConnectInfo(id, "[" + addr.toString() + "][Established]");
      proxynegotiation = false;
      if (implicittls) {
        global->getIOManager()->negotiateSSLConnect(sockid);
      }
      break;
    case PROXYSESSION_ERROR:
      owner->ftpConnectInfo(id, "[" + addr.toString() + "][Proxy error: " + proxysession->getErrorMessage() + "]");
      disengage();
      owner->ftpConnectInfo(id, "[" + addr.toString() + "][Disconnected]");
      owner->ftpConnectFail(id);
      break;
  }
}

Address FTPConnect::getAddress() const {
  return addr;
}

bool FTPConnect::isPrimary() const {
  return primary;
}

void FTPConnect::disengage() {
  if (engaged) {
    global->getIOManager()->closeSocket(sockid);
    engaged = false;
  }
}

void FTPConnect::tickIntern() {
  millisecs += 1000;
  if (millisecs >= WELCOME_TIMEOUT_MSEC) {
    if (engaged && connected && !welcomereceived) {
      owner->ftpConnectInfo(id, "[" + addr.toString() + "][Timeout while waiting for welcome message]");
      disengage();
      owner->ftpConnectFail(id);
    }
  }
}
