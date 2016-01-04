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
  void FDConnected(int);
  void FDData(int, char *, unsigned int);
  void FDFail(int, std::string);
  int getId() const;
  int handOver();
  std::string getAddress() const;
  std::string getPort() const;
  bool isPrimary() const;
private:
  void proxySessionInit();
  int id;
  int sockid;
  bool proxynegotiation;
  ProxySession * proxysession;
  FTPConnectOwner * owner;
  char * databuf;
  unsigned int databuflen;
  unsigned int databufpos;
  int databufcode;
  bool handover;
  std::string addr;
  std::string port;
  bool primary;
};
