#pragma once

#include <memory>
#include <string>

#include "core/eventreceiver.h"

#include "address.h"

class FTPConnectOwner;
class ProxySession;
class Proxy;
struct Address;

class FTPConnect : public Core::EventReceiver {
public:
  FTPConnect(int, FTPConnectOwner *, const Address& addr, Proxy *, bool, bool implicittls);
  ~FTPConnect();
  int getId() const;
  int handedOver();
  Address getAddress() const;
  bool isPrimary() const;
  void disengage();
  void tickIntern();
private:
  void FDConnecting(int sockid, const std::string& addr) override;
  void FDConnected(int sockid) override;
  void FDDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) override;
  void FDData(int sockid, char* data, unsigned int datalen) override;
  void FDFail(int sockid, const std::string& error) override;
  void FDSSLSuccess(int sockid, const std::string& cipher) override;
  void proxySessionInit();
  void printConnecting(const Address& addr, bool resolved = false);
  int id;
  int sockid;
  bool proxynegotiation;
  ProxySession * proxysession;
  FTPConnectOwner * owner;
  unsigned int databuflen;
  char * databuf;
  unsigned int databufpos;
  int databufcode;
  Address addr;
  Address resolvedaddr;
  Proxy * proxy;
  bool primary;
  bool engaged;
  bool connected;
  bool welcomereceived;
  int millisecs;
  bool implicittls;
};
