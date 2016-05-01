#include "localupload.h"

#include <cstring>
#include <stdlib.h>
#include <unistd.h>

#include "core/iomanager.h"
#include "core/tickpoke.h"
#include "globalcontext.h"
#include "eventlog.h"
#include "transfermonitor.h"
#include "ftpconn.h"
#include "localstorage.h"
#include "util.h"

extern GlobalContext * global;

LocalUpload::LocalUpload() :
  filepos(0)
{
  buf = (char *) malloc(CHUNK);
  buflen = CHUNK;
}

void LocalUpload::engage(TransferMonitor * tm, const std::string & path, const std::string & filename, const std::string & addr, int port, bool ssl, FTPConn * ftpconn) {
  init(tm, ftpconn, path, filename, ssl, port, true);
  sockid = global->getIOManager()->registerTCPClientSocket(this, addr, port);
}

bool LocalUpload::engage(TransferMonitor * tm, const std::string & path, const std::string & filename, int port, bool ssl, FTPConn * ftpconn) {
  init(tm, ftpconn, path, filename, ssl, port, false);
  sockid = global->getIOManager()->registerTCPServerSocket(this, port);
  return sockid != -1;
}

void LocalUpload::init(TransferMonitor * tm, FTPConn * ftpconn, const std::string & path, const std::string & filename, bool ssl, int port, bool passivemode) {
  this->tm = tm;
  this->ftpconn = ftpconn;
  this->path = path;
  this->filename = filename;
  this->ssl = ssl;
  this->port = port;
  this->passivemode = passivemode;
  filepos = 0;
  inuse = true;
  fileopened = false;
  if (!passivemode) {
    global->getTickPoke()->startPoke(this, "LocalUpload", 5000, 0);
  }
}

bool LocalUpload::active() const {
  return inuse;
}

void LocalUpload::FDConnected(int sockid) {
  if (!passivemode) {
    tm->activeStarted();
  }
  openFile(R_OK);
  if (ssl) {
    global->getIOManager()->negotiateSSLConnect(sockid, (EventReceiver *)ftpconn);
  }
  else {
    sendChunk();
  }
}

void LocalUpload::FDDisconnected(int sockid) {
  if (fileopened) {
    filestream.close();
  }
  inuse = false;
  tm->sourceError(TM_ERR_OTHER);
}

void LocalUpload::FDSSLSuccess(int sockid) {
  ftpconn->printCipher(sockid);
  sendChunk();
}

void LocalUpload::sendChunk() {
  util::assert(fileopened);
  filestream.read(buf, buflen);
  int gcount = filestream.gcount();
  if (gcount == 0) {
    filestream.close();
    global->getIOManager()->closeSocket(sockid);
    tm->sourceComplete();
    inuse = false;
  }
  filepos += gcount;
  global->getIOManager()->sendData(sockid, buf, gcount);
}

void LocalUpload::FDSendComplete(int sockid) {
  sendChunk();
}

void LocalUpload::FDSSLFail(int sockid) {
  if (fileopened) { // this can theoretically happen mid-transfer
    filestream.close();
  }
  global->getIOManager()->closeSocket(sockid);
  inuse = false;
  tm->sourceError(TM_ERR_OTHER);
}

void LocalUpload::FDFail(int sockid, std::string error) {
  inuse = false;
  if (sockid != -1) {
    tm->sourceError(TM_ERR_OTHER);
  }
}

void LocalUpload::FDData(int sockid, char * data, unsigned int len) {
  if (fileopened) {
    filestream.close();
  }
  inuse = false;
  global->getIOManager()->closeSocket(sockid);
  tm->sourceError(TM_ERR_OTHER);
}

unsigned long long int LocalUpload::size() const {
  return filepos;
}
