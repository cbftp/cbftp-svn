#pragma once

#include <string>

#include "core/eventreceiver.h"
#include "core/pointer.h"

class FTPConnectOwner;
class ProxySession;
class Proxy;

class FTPConnect : public EventReceiver {
public:
  FTPConnect(int, FTPConnectOwner *, const std::string &, const std::string &, Proxy *, bool);
  ~FTPConnect();
  void FDConnecting(int, const std::string &);
  void FDConnected(int);
  void FDDisconnected(int);
  void FDData(int, char *, unsigned int);
  void FDFail(int, const std::string &);
  int getId() const;
  int handedOver();
  std::string getAddress() const;
  std::string getPort() const;
  bool isPrimary() const;
  void disengage();
  void tickIntern();
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
  bool connected;
  bool welcomereceived;
  int millisecs;
};
