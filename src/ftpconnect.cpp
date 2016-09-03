#include "ftpconnect.h"

#include "core/iomanager.h"
#include "site.h"
#include "ftpconnectowner.h"
#include "ftpconn.h"
#include "globalcontext.h"
#include "proxysession.h"
#include "proxy.h"
#include "util.h"

#define WELCOME_TIMEOUT_MSEC 5000

FTPConnect::FTPConnect(int id, FTPConnectOwner * owner, const std::string & addr, const std::string & port, Proxy * proxy, bool primary) :
  id(id),
  sockid(-1),
  proxynegotiation(false),
  proxysession(new ProxySession()),
  owner(owner),
  databuflen(DATABUF),
  databuf((char *) malloc(databuflen)),
  databufpos(0),
  addr(addr),
  port(port),
  proxy(proxy),
  primary(primary),
  engaged(true),
  connected(false),
  welcomereceived(false),
  millisecs(0)
{
  bool resolving;
  if (proxy == NULL) {
    sockid = global->getIOManager()->registerTCPClientSocket(this, addr, util::str2Int(port), resolving);
    if (resolving) {
      owner->ftpConnectInfo(id, "[Resolving " + addr + "]");
    }
    else {
      FDConnecting(sockid, addr);
    }
  }
  else {
    proxynegotiation = true;
    proxysession->prepare(proxy, addr, port);
    sockid = global->getIOManager()->registerTCPClientSocket(this, proxy->getAddr(), util::str2Int(proxy->getPort()), resolving);
    if (resolving) {
      owner->ftpConnectInfo(id, "[Resolving proxy " + proxy->getAddr() + "]");
    }
    else {
      FDConnecting(sockid, proxy->getAddr());
    }
  }
}

FTPConnect::~FTPConnect() {
  util::assert(!engaged); // must disengage before deleting; events may still be in the work queue
  delete proxysession;
  free(databuf);
}

void FTPConnect::FDConnecting(int sockid, std::string addr) {
  if (proxynegotiation) {
    owner->ftpConnectInfo(id, "[Connecting to proxy " + addr + ":" + proxy->getPort() + "]");
  }
  else {
    owner->ftpConnectInfo(id, "[Connecting to " + addr + ":" + port + "]");
  }
}

void FTPConnect::FDConnected(int sockid) {
  connected = true;
  millisecs = 0;
  owner->ftpConnectInfo(id, "[Connection established]");
  if (proxynegotiation) {
    proxySessionInit();
  }
}

void FTPConnect::FDDisconnected(int sockid) {
  FDFail(sockid, "Disconnected");
}

void FTPConnect::FDData(int sockid, char * data, unsigned int datalen) {
  if (proxynegotiation) {
    proxysession->received(data, datalen);
    proxySessionInit();
  }
  else {
    owner->ftpConnectInfo(id, std::string(data, datalen));
    if (FTPConn::parseData(data, datalen, &databuf, databuflen, databufpos, databufcode)) {
      if (databufcode == 220) {
        welcomereceived = true;
        owner->ftpConnectSuccess(id);
      }
      else {
        owner->ftpConnectInfo(id, "[Unknown response]");
        disengage();
        owner->ftpConnectInfo(id, "[Disconnected]");
        owner->ftpConnectFail(id);
      }
    }
  }
}

void FTPConnect::FDFail(int sockid, std::string error) {
  engaged = false;
  owner->ftpConnectInfo(id, "[" + error + "]");
  owner->ftpConnectFail(id);
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
      owner->ftpConnectInfo(id, "[Connecting to " + addr + ":" + port + " through proxy]");
      global->getIOManager()->sendData(sockid, proxysession->getSendData(), proxysession->getSendDataLen());
      break;
    case PROXYSESSION_SEND:
      global->getIOManager()->sendData(sockid, proxysession->getSendData(), proxysession->getSendDataLen());
      break;
    case PROXYSESSION_SUCCESS:
      owner->ftpConnectInfo(id, "[Connection established]");
      proxynegotiation = false;
      break;
    case PROXYSESSION_ERROR:
      owner->ftpConnectInfo(id, "[Proxy error: " + proxysession->getErrorMessage() + "]");
      disengage();
      owner->ftpConnectInfo(id, "[Disconnected]");
      owner->ftpConnectFail(id);
      break;
  }
}

std::string FTPConnect::getAddress() const {
  return addr;
}

std::string FTPConnect::getPort() const {
  return port;
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

void FTPConnect::tick() {
  millisecs += 1000;
  if (millisecs >= WELCOME_TIMEOUT_MSEC) {
    if (engaged && connected && !welcomereceived) {
      owner->ftpConnectInfo(id, "[Timeout while waiting for welcome message]");
      disengage();
      owner->ftpConnectFail(id);
    }
  }
}
