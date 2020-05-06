#pragma once

#include <memory>

#include "core/eventreceiver.h"

#include "address.h"

class Proxy;
class ProxySession;

class EventReceiverProxyIntermediate : public Core::EventReceiver {
public:
  EventReceiverProxyIntermediate();
  ~EventReceiverProxyIntermediate();
  void interConnect(const Address& addr, Proxy* proxy = nullptr, bool listenimmediately = true);
  virtual void FDInterConnecting(int sockid, const std::string& addr);
  virtual void FDInterInfo(int sockid, const std::string& info);
  virtual void FDInterConnected(int sockid) = 0;
  virtual void FDInterData(int sockid, char* data, unsigned int datalen) = 0;
protected:
  void negotiateSSLConnect();
  int sockid;
private:
  void FDConnecting(int sockid, const std::string& addr) final override;
  void FDConnected(int sockid) final override;
  void FDData(int sockid, char* data, unsigned int datalen) final override;
  void proxySessionInit();
  Proxy* proxy;
  Address addr;
  std::unique_ptr<ProxySession> proxysession;
  bool proxynegotiation;
};
