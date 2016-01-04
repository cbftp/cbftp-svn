#include "proxysession.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstring>

#include "proxy.h"
#include "util.h"

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

std::string getConnectError(char c) {
  switch (c) {
    case 1:
      return "General failure";
    case 2:
      return "Connection not allowed by ruleset";
    case 3:
      return "Network unreachable";
    case 4:
      return "Host unreachable";
    case 5:
      return "Connection refused by destination host";
    case 6:
      return "TTL expired";
    case 7:
      return "Command not supported / protocol error";
    case 8:
      return "Address type not supported";
  }
  return "Unknown error code: " + util::int2Str(c);
}

ProxySession::ProxySession() :
  state(PROXYSESSION_INIT)
{
}

void ProxySession::prepare(Proxy * proxy, std::string addr, std::string port) {
  this->proxy = proxy;
  state = PROXYSESSION_INIT;
  senddata[0] = PROXYSESSION_SOCKSVERSION;
  senddata[1] = PROXYSESSION_AUTHTYPESSUPPORTED;
  if (proxy->getAuthMethod() == PROXY_AUTH_NONE) {
    senddata[2] = PROXYSESSION_AUTH_NONE;
  }
  else {
    senddata[2] = PROXYSESSION_AUTH_USERPASS;
  }
  senddatalen = 3;
  struct addrinfo sock, *res;
  memset(&sock, 0, sizeof(sock));
  sock.ai_family = AF_INET;
  sock.ai_socktype = SOCK_STREAM;
  int status = getaddrinfo(addr.data(), port.data(), &sock, &res);
  if (status != 0) {
    state = PROXYSESSION_ERROR;
    errormessage = "Failed to retrieve address information";
    return;
  }
  saddr = (struct sockaddr_in*)res->ai_addr;
}

int ProxySession::instruction() const {
  if (state == PROXYSESSION_INIT) {
    return PROXYSESSION_SEND;
  }
  return state;
}

const char * ProxySession::getSendData() const {
  return senddata;
}

int ProxySession::getSendDataLen() const {
  return senddatalen;
}

void ProxySession::received(char * data, int datalen) {
  switch (state) {
    case PROXYSESSION_INIT:
      if (datalen < 2) {
        state = PROXYSESSION_ERROR;
        errormessage = "Malformed response on session request";
        break;
      }
      if (data[0] != PROXYSESSION_SOCKSVERSION) {
        state = PROXYSESSION_ERROR;
        errormessage = "Invalid SOCKS version in response";
        break;
      }
      if (data[1] != senddata[2]) {
        state = PROXYSESSION_ERROR;
        errormessage = "Unsupported authentication type";
        break;
      }
      if (data[1] == PROXYSESSION_AUTH_NONE) {
        state = PROXYSESSION_SEND_CONNECT;
        setConnectRequestData();
        break;
      }
      state = PROXYSESSION_SEND;
      senddata[0] = PROXYSESSION_AUTH_VERSION;
      if (proxy->getAuthMethod() == PROXY_AUTH_USERPASS) {
        int pos = proxy->getUser().length();
        senddata[1] = (char) pos;
        memcpy((void *) &senddata[2], proxy->getUser().c_str(), pos);
        pos = pos + 2;
        int passlen = proxy->getPass().length();
        senddata[pos] = (char) passlen;
        pos = pos + 1;
        memcpy((void *) &senddata[pos], proxy->getPass().c_str(), passlen);
        senddatalen = pos + passlen;
      }
      break;
    case PROXYSESSION_SEND:
      if (datalen < 2) {
        state = PROXYSESSION_ERROR;
        errormessage = "Malformed response on authentication request";
        break;
      }
      if (data[0] != PROXYSESSION_AUTH_VERSION) {
        state = PROXYSESSION_ERROR;
        errormessage = "Invalid authentication version";
        break;
      }
      if (data[1] != PROXYSESSION_STATUS_SUCCESS) {
        state = PROXYSESSION_ERROR;
        errormessage = "Authentication failed";
        break;
      }
      state = PROXYSESSION_SEND_CONNECT;
      setConnectRequestData();
      break;
    case PROXYSESSION_SEND_CONNECT:
      if (datalen < 10) {
        state = PROXYSESSION_ERROR;
        errormessage = "Malformed response on connect request";
        break;
      }
      if (data[0] != PROXYSESSION_SOCKSVERSION) {
        state = PROXYSESSION_ERROR;
        errormessage = "Invalid SOCKS version in response";
        break;
      }
      if (data[1] != PROXYSESSION_STATUS_SUCCESS) {
        state = PROXYSESSION_ERROR;
        errormessage = getConnectError(data[1]);
        break;
      }
      if (data[3] != PROXYSESSION_ADDR_IPV4) {
        state = PROXYSESSION_ERROR;
        errormessage = "Invalid address type in response";
        break;
      }
      state = PROXYSESSION_SUCCESS;
      break;
  }
}

void ProxySession::setConnectRequestData() {
  senddata[0] = PROXYSESSION_SOCKSVERSION;
  senddata[1] = PROXYSESSION_TCP_STREAM;
  senddata[2] = PROXYSESSION_RESERVED;
  senddata[3] = PROXYSESSION_ADDR_IPV4;
  memcpy((void *) &senddata[4], &saddr->sin_addr.s_addr, 4);
  memcpy((void *) &senddata[8], &saddr->sin_port, 2);
  senddatalen = 10;
}

std::string ProxySession::getErrorMessage() const {
  return errormessage;
}
