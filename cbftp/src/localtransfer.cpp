#include "localtransfer.h"

#include <cstdio>
#include <cerrno>

#include "core/iomanager.h"
#include "core/tickpoke.h"
#include "globalcontext.h"
#include "transfermonitor.h"
#include "filesystem.h"

LocalTransfer::LocalTransfer() :
  buflen(0),
  timeoutticker(false),
  inuse(false)
{
}

bool LocalTransfer::active() const {
  return inuse;
}

void LocalTransfer::FDNew(int sockid, int newsockid) {
  global->getIOManager()->closeSocket(sockid);
  if (timeoutticker) {
    global->getTickPoke()->stopPoke(this, 0);
    timeoutticker = false;
  }
  this->sockid = newsockid;
  global->getIOManager()->registerTCPServerClientSocket(this, newsockid);
  FDConnected(newsockid);
}

void LocalTransfer::tick(int) {
  global->getIOManager()->closeSocket(sockid);
  global->getTickPoke()->stopPoke(this, 0);
  FDFail(sockid, "Connection timed out");
}

void LocalTransfer::openFile(bool read) {
  if (read ? !FileSystem::fileExistsReadable(path) : !FileSystem::fileExistsWritable(path)) {
    perror(std::string("There was an error accessing " + path.toString()).c_str());
    exit(1);
  }
  filestream.clear();
  filestream.open((path / filename).toString().c_str(), std::ios::binary | (read ? std::ios::in : (std::ios::ate | std::ios::out)));
  fileopened = true;
}

int LocalTransfer::getPort() const {
  return port;
}

FTPConn * LocalTransfer::getConn() const {
  return ftpconn;
}

void LocalTransfer::activate() {
  inuse = true;
  timeoutticker = false;
  if (!passivemode) {
    global->getTickPoke()->startPoke(this, "LocalTransfer", 5000, 0);
    timeoutticker = true;
  }
}

void LocalTransfer::deactivate() {
  inuse = false;
  if (timeoutticker) {
    global->getTickPoke()->stopPoke(this, 0);
    timeoutticker = false;
  }
}
