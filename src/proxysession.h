#pragma once

#include <string>

enum ProxySessionStatus {
  PROXYSESSION_INIT,
  PROXYSESSION_SEND,
  PROXYSESSION_ERROR,
  PROXYSESSION_SUCCESS,
  PROXYSESSION_SEND_CONNECT,
};

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
