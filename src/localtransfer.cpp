#include "localtransfer.h"

#include <stdio.h>
#include <errno.h>

#include "core/iomanager.h"
#include "core/tickpoke.h"
#include "globalcontext.h"
#include "transfermonitor.h"

extern GlobalContext * global;

LocalTransfer::LocalTransfer() :
  inuse(false),
  buflen(0)
{
}

bool LocalTransfer::active() const {
  return inuse;
}

void LocalTransfer::FDNew(int sockid) {
  global->getIOManager()->closeSocket(this->sockid);
  if (!passivemode) {
    global->getTickPoke()->stopPoke(this, 0);
  }
  this->sockid = sockid;
  global->getIOManager()->registerTCPServerClientSocket(this, sockid);
  FDConnected(sockid);
}

void LocalTransfer::tick(int) {
  global->getIOManager()->closeSocket(sockid);
  if (!passivemode) {
    global->getTickPoke()->stopPoke(this, 0);
  }
  FDFail(sockid, "Connection timed out");
}

void LocalTransfer::openFile(bool read) {
  if (access(path.c_str(), read ? R_OK : (R_OK | W_OK)) < 0) {
    perror(std::string("There was an error accessing " + path).c_str());
    exit(1);
  }
  filestream.clear();
  filestream.open((path + "/" + filename).c_str(), std::ios::binary | (read ? std::ios::in : (std::ios::ate | std::ios::out)));
  fileopened = true;
}

int LocalTransfer::getPort() const {
  return port;
}

FTPConn * LocalTransfer::getConn() const {
  return ftpconn;
}
