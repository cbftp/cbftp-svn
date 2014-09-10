#pragma once

#include <string>
#include <list>
#include <openssl/ssl.h>

#include "datablock.h"

#define FD_UNUSED 1030
#define FD_KEYBOARD 1031
#define FD_TCP_PLAIN 1032
#define FD_TCP_SSL_NEG_REDO_CONN 1033
#define FD_TCP_SSL_NEG_REDO_WRITE 1034
#define FD_TCP_SSL 1035
#define FD_UDP 1036
#define FD_TCP_SERVER 1037
#define FD_TCP_PLAIN_LISTEN 1038
#define FD_TCP_SSL_NEG_REDO_ACCEPT 1039
#define FD_TCP_SSL_NEG_REDO_HANDSHAKE 1040
#define FD_TCP_CONNECTING 1041

class EventReceiver;

class SocketInfo {
public:
  SocketInfo() : type(FD_UNUSED), fd(0), receiver(NULL), ssl(NULL) {
  }
  int type;
  int fd;
  std::string addr;
  EventReceiver * receiver;
  std::list<DataBlock> sendqueue;
  SSL * ssl;
};
