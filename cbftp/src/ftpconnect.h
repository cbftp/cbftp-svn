#pragma once

#include <memory>
#include <string>

#include "core/eventreceiver.h"

class FTPConnectOwner;
class ProxySession;
class Proxy;

class FTPConnect : public EventReceiver {
public:
  FTPConnect(int, FTPConnectOwner *, const std::string &, const std::string &, Proxy *, bool, bool implicittls);
  ~FTPConnect();
  void FDConnecting(int, const std::string &);
  void FDConnected(int);
  void FDDisconnected(int);
  void FDData(int, char *, unsigned int);
  void FDFail(int, const std::string &);
  void FDSSLSuccess(int sockid, const std::string & cipher);
  void FDSSLFail(int sockid);
  int getId() const;
  int handedOver();
  std::string getAddress() const;
  std::string getPort() const;
  bool isPrimary() const;
  void disengage();
  void tickIntern();
private:
  void proxySessionInit();
  void printConnecting(const std::string & addr, bool resolved = false);
  int id;
  int sockid;
  bool proxynegotiation;
  ProxySession * proxysession;
  FTPConnectOwner * owner;
  unsigned int databuflen;
  char * databuf;
  unsigned int databufpos;
  int databufcode;
  std::string addr;
  std::string port;
  Proxy * proxy;
  bool primary;
  bool engaged;
  bool connected;
  bool welcomereceived;
  int millisecs;
  bool implicittls;
};
