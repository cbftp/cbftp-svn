#include "eventreceiverproxyintermediate.h"

#include "globalcontext.h"
#include "core/iomanager.h"

#include "proxy.h"
#include "proxysession.h"

EventReceiverProxyIntermediate::EventReceiverProxyIntermediate() : sockid(-1), proxy(nullptr) {

}

EventReceiverProxyIntermediate::~EventReceiverProxyIntermediate() {

}

void EventReceiverProxyIntermediate::interConnect(const Address& addr, Proxy* proxy, bool listenimmediately) {
  this->proxy = proxy;
  this->addr = addr;
  proxynegotiation = proxy;
  bool resolving;
  if (proxy == nullptr) {
    sockid = global->getIOManager()->registerTCPClientSocket(this, addr.host, addr.port, resolving, addr.addrfam, listenimmediately);
    if (resolving) {
      FDInterInfo(sockid, "Resolving");
    }
    else {
      FDInterInfo(sockid, "Connecting to " + addr.toString(false));
    }
  }
  else {
    proxysession = std::unique_ptr<ProxySession>(new ProxySession());
    proxysession->prepare(proxy, addr.host, std::to_string(addr.port));
    sockid = global->getIOManager()->registerTCPClientSocket(this, proxy->getAddr(), std::stoi(proxy->getPort()), resolving);
    if (resolving) {
      FDInterInfo(sockid, "Resolving proxy " + proxy->getAddr());
    }
    else {
      Address proxyaddr;
      proxyaddr.host = proxy->getAddr();
      proxyaddr.port = std::stoi(proxy->getPort());
      FDInterInfo(sockid, "Connecting to proxy " + proxyaddr.toString(false));
    }
  }
}

void EventReceiverProxyIntermediate::FDInterConnecting(int sockid, const std::string& addr) {

}

void EventReceiverProxyIntermediate::FDInterInfo(int sockid, const std::string& info) {

}

void EventReceiverProxyIntermediate::FDConnecting(int sockid, const std::string& addr) {
  Address resolvedaddr;
  resolvedaddr.host = addr;
  resolvedaddr.brackets = addr.find(":") != std::string::npos && resolvedaddr.port != 21;
  FDInterInfo(sockid, std::string("Connecting to ") + (proxy ? "proxy " : "") + resolvedaddr.toString(false));
  if (!proxy) {
    FDInterConnecting(sockid, addr);
  }
}

void EventReceiverProxyIntermediate::FDConnected(int sockid) {
  if (proxynegotiation) {
    FDInterInfo(sockid, "Proxy connection established");
    proxySessionInit();
  }
  else {
    FDInterConnected(sockid);
  }
}

void EventReceiverProxyIntermediate::FDData(int sockid, char* data, unsigned int datalen) {
  if (proxynegotiation) {
    proxysession->received(data, datalen);
    proxySessionInit();
  }
  else {
    FDInterData(sockid, data, datalen);
  }
}

void EventReceiverProxyIntermediate::proxySessionInit() {
  switch (proxysession->instruction()) {
    case PROXYSESSION_SEND_CONNECT:
      FDInterInfo(sockid, "Connecting through proxy");
      global->getIOManager()->sendData(sockid, proxysession->getSendData(), proxysession->getSendDataLen());
      break;
    case PROXYSESSION_SEND:
      global->getIOManager()->sendData(sockid, proxysession->getSendData(), proxysession->getSendDataLen());
      break;
    case PROXYSESSION_SUCCESS:
      proxynegotiation = false;
      FDInterConnected(sockid);
      break;
    case PROXYSESSION_ERROR:
      global->getIOManager()->closeSocket(sockid);
      FDDisconnected(sockid, Core::DisconnectType::ERROR, "Proxy error: " + proxysession->getErrorMessage());
      break;
  }
}

void EventReceiverProxyIntermediate::negotiateSSLConnect() {
  global->getIOManager()->negotiateSSLConnect(sockid);
}
