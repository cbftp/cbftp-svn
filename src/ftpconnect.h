#pragma once

#include <string>

#include "eventreceiver.h"
#include "pointer.h"

class FTPConnectOwner;
class ProxySession;
class Proxy;

class FTPConnect : public EventReceiver {
public:
  FTPConnect(int, FTPConnectOwner *, const std::string &, const std::string &, Proxy *, bool);
  ~FTPConnect();
  void FDConnecting(int, std::string);
  void FDConnected(int);
  void FDData(int, char *, unsigned int);
  void FDFail(int, std::string);
  int getId() const;
  int handedOver();
  std::string getAddress() const;
  std::string getPort() const;
  bool isPrimary() const;
  void disengage();
private:
  void proxySessionInit();
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
};
