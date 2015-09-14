#pragma once

#include <string>

#define PROXYSESSION_SEND 336
#define PROXYSESSION_ERROR 337
#define PROXYSESSION_SUCCESS 338
#define PROXYSESSION_SEND_CONNECT 339

#define PROXYSESSION_SOCKSVERSION 5
#define PROXYSESSION_AUTHTYPESSUPPORTED 1
#define PROXYSESSION_AUTH_NONE 0
#define PROXYSESSION_AUTH_USERPASS 2
#define PROXYSESSION_AUTH_VERSION 1
#define PROXYSESSION_STATUS_SUCCESS 0
#define PROXYSESSION_TCP_STREAM 1
#define PROXYSESSION_TCP_BIND 2
#define PROXYSESSION_UDP 3
#define PROXYSESSION_RESERVED 0
#define PROXYSESSION_ADDR_IPV4 1
#define PROXYSESSION_ADDR_DNS 3
#define PROXYSESSION_ADDR_IPV6 4

class Proxy;

class ProxySession {
public:
  ProxySession();
  void prepare(Proxy *, std::string, std::string);
  int instruction() const;
  const char * getSendData() const;
  int getSendDataLen() const;
  void received(char *, int);
  std::string getErrorMessage() const;
private:
  void setConnectRequestData();
  struct sockaddr_in* saddr;
  char senddata[256];
  int senddatalen;
  Proxy * proxy;
  int state;
  std::string errormessage;
};
