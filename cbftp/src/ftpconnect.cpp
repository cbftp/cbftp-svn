#include "ftpconnect.h"

#include "site.h"
#include "ftpconnectowner.h"
#include "ftpconn.h"

#include "iomanager.h"
#include "globalcontext.h"
#include "proxysession.h"
#include "proxy.h"
#include "util.h"

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
  primary(primary),
  engaged(true)
{
  if (proxy == NULL) {
    owner->ftpConnectInfo(id, "[Connecting to " + addr + ":" + port + "]");
    global->getIOManager()->registerTCPClientSocket(this, addr, util::str2Int(port), &sockid);
  }
  else {
    owner->ftpConnectInfo(id, "[Connecting to proxy " + proxy->getAddr() + ":" + proxy->getPort() + "]");
    proxynegotiation = true;
    proxysession->prepare(proxy, addr, port);
    global->getIOManager()->registerTCPClientSocket(this, proxy->getAddr(), util::str2Int(proxy->getPort()), &sockid);
  }
}

FTPConnect::~FTPConnect() {
  util::assert(!engaged); // must disengage before deleting; events may still be in the work queue
  delete proxysession;
  free(databuf);
}

void FTPConnect::FDConnected(int sockid) {
  owner->ftpConnectInfo(id, "[Connection established]");
  if (proxynegotiation) {
    proxySessionInit();
  }
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
