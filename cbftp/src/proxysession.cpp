#include "proxysession.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstring>

#include "proxy.h"

ProxySession::ProxySession() {
  state = 0;
}

void ProxySession::prepare(Proxy * proxy, std::string addr, std::string port) {
  this->proxy = proxy;
  state = 0;
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
    state = 13;
    return;
  }
  saddr = (struct sockaddr_in*)res->ai_addr;
}

int ProxySession::instruction() const {
  switch (state) {
    case 0:
    case 1:
      return PROXYSESSION_SEND;
    case 2:
      return PROXYSESSION_SEND_CONNECT;
    case 3:
      return PROXYSESSION_SUCCESS;
    case 13:
      return PROXYSESSION_ERROR;
  }
  return PROXYSESSION_ERROR;
}

const char * ProxySession::getSendData() const {
  return senddata;
}

int ProxySession::getSendDataLen() const {
  return senddatalen;
}

void ProxySession::received(char * data, int datalen) {
  switch (state) {
    case 0:
      if (datalen < 2) {
        state = 13;
        errormessage = "Malformed response on session request";
        break;
      }
      if (data[0] != PROXYSESSION_SOCKSVERSION) {
        state = 13;
        errormessage = "Invalid SOCKS version in response";
        break;
      }
      if (data[1] != senddata[2]) {
        state = 13;
        errormessage = "Unsupported authentication type";
        break;
      }
      if (data[1] == PROXYSESSION_AUTH_NONE) {
        state = 2;
        setConnectRequestData();
        break;
      }
      state = 1;
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
    case 1:
      if (datalen < 2) {
        state = 13;
        errormessage = "Malformed response on authentication request";
        break;
      }
      if (data[0] != PROXYSESSION_AUTH_VERSION) {
        state = 13;
        errormessage = "Invalid authentication version";
        break;
      }
      if (data[1] != PROXYSESSION_STATUS_SUCCESS) {
        state = 13;
        errormessage = "Authentication failed";
        break;
      }
      state = 2;
      setConnectRequestData();
      break;
    case 2:
      if (datalen < 10) {
        state = 13;
        errormessage = "Malformed response on connect request";
        break;
      }
      if (data[0] != PROXYSESSION_SOCKSVERSION) {
        state = 13;
        errormessage = "Invalid SOCKS version in response";
        break;
      }
      if (data[1] != PROXYSESSION_STATUS_SUCCESS) {
        state = 13;
        break;
      }
      if (data[3] != PROXYSESSION_ADDR_IPV4) {
        state = 13;
        errormessage = "Invalid address type in response";
        break;
      }
      state = 3;
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
